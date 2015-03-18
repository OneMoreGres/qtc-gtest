#include <QDebug>
#include <QFileInfo>
#include <QRegularExpression>

#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/idocument.h>
#include <cpptools/cppmodelmanager.h>
#include <cpptools/searchsymbols.h>
#include <cplusplus/DependencyTable.h>
#include <projectexplorer/project.h>
#include <projectexplorer/session.h>
#include <projectexplorer/projectexplorer.h>
#include <projectexplorer/target.h>

#include "TestProject.h"
#include "CustomRunConfiguration.h"

using namespace QtcGtest::Internal;

using namespace Core;
using namespace ProjectExplorer;
using namespace CppTools;
using namespace CPlusPlus;

namespace
{
  const QString gtestInclude = QLatin1String ("gtest.h");
  const QString gtestFilter = QLatin1String ("--gtest_filter=%1");
  const QString gtestFilterSeparator = QLatin1String (":");
  const QRegularExpression testPattern (
      QLatin1String ("(TEST|TEST_F|TYPED_TEST|TYPED_TEST_P|TEST_P)\\s*\\((\\w+),\\s*\\w+\\)"));
  enum Test {TestType = 1, TestCase, TestName};


  QString shortenFileName (const QString& file)
  {
    QFileInfo f (file);
    QString newName = f.absolutePath () + QLatin1Char ('/') + f.baseName ();
    return newName;
  }
}

TestProject::TestProject(QObject *parent) :
  QObject(parent)
{
  testFilterPatterns_[QLatin1String ("TYPED_TEST")] = QLatin1String ("%1/*.*");
  testFilterPatterns_[QLatin1String ("TYPED_TEST_P")] = QLatin1String ("*/%1/*.*");
  testFilterPatterns_[QLatin1String ("TEST_P")] = QLatin1String ("*/%1.*");
  testFilterPatterns_[QLatin1String ("TEST")] = QLatin1String ("%1.*");
  testFilterPatterns_[QLatin1String ("TEST_F")] = QLatin1String ("%1.*");
}

void TestProject::checkProject()
{
  Project* project = SessionManager::startupProject ();
  CustomRunConfiguration* configuration = parse (project);
  if (configuration != NULL)
  {
    runTests (configuration);
  }
}

void TestProject::runTests(CustomRunConfiguration *configuration) const
{
  Q_ASSERT (configuration != NULL);
  ProjectExplorerPlugin* plugin = ProjectExplorerPlugin::instance ();
  plugin->runRunConfiguration (configuration, NormalRunMode, true);
  configuration->deleteLater ();
}

void TestProject::checkChanged()
{
  if (changedFiles_.isEmpty ())
  {
    return;
  }
  Project* project = SessionManager::startupProject ();
  CustomRunConfiguration* configuration = parse (project);
  if (configuration != NULL)
  {
    runTestsForFiles (changedFiles_, configuration);
    changedFiles_.clear ();
  }
}

void TestProject::checkCurrent()
{
  Project* project = SessionManager::startupProject ();
  CustomRunConfiguration* configuration = parse (project);
  if (configuration == NULL)
  {
    return;
  }

  IDocument* document = EditorManager::currentDocument ();
  if (document == NULL)
  {
    return;
  }
  QString file = document->filePath ().toString ();
  QStringList files = project->files (Project::ExcludeGeneratedFiles);
  if (!files.contains (file))
  {
    return;
  }

  runTestsForFiles (QStringList () << file, configuration);
}

void TestProject::runTestsForFiles(const QStringList &files, CustomRunConfiguration *configuration) const
{
  Q_ASSERT (configuration != NULL);
  Q_ASSERT (!gtestIncludeFiles_.isEmpty ());
  QSet<QString> testFiles = getDependentFiles (gtestIncludeFiles_).toSet ();
  QSet<QString> dependentFiles = getDependentFiles (files).toSet ();
  testFiles.intersect (dependentFiles);
  if (testFiles.isEmpty ())
  {
    return;
  }

  QStringList testCases = getTestCases (testFiles);
  if (testCases.isEmpty ())
  {
    return;
  }
  QString arguments = gtestFilter.arg(testCases.join (gtestFilterSeparator));
  configuration->setArguments (arguments);
  runTests (configuration);
}

QStringList TestProject::getDependentFiles(const QStringList &files) const
{
  QStringList dependentFiles;
  QStringList uncheckedFiles = files;
  while (!uncheckedFiles.isEmpty ())
  {
    QString file = uncheckedFiles.takeFirst ();
    if (dependentFiles.contains (file))
    {
      continue;
    }
    dependentFiles << file;
    QString newName = shortenFileName (file);
    QStringList newFiles = dependencyTable_.value (newName);
    if (!newFiles.isEmpty ())
    {
      dependentFiles += newFiles;
      uncheckedFiles += newFiles;
    }
  }
  dependentFiles.removeDuplicates ();
  return dependentFiles;
}

QStringList TestProject::getTestCases(const QSet<QString> &fileNames) const
{
  QStringList testCases;
  foreach (const QString& file, fileNames)
  {
    QFile f (file);
    if (!f.open (QFile::ReadOnly))
    {
      continue;
    }
    //TODO Support codecs?
    QString source = QString::fromLocal8Bit (f.readAll ());
    f.close ();
    QRegularExpressionMatchIterator i = testPattern.globalMatch (source);
    while (i.hasNext())
    {
      QRegularExpressionMatch match = i.next();
      QString caseType = match.captured (TestType);
      Q_ASSERT(testFilterPatterns_.contains(caseType));
      QString pattern = testFilterPatterns_[caseType].arg (match.captured (TestCase));
      testCases << pattern;
    }
  }
  testCases.removeDuplicates ();
  return testCases;
}

void TestProject::handleDocumentsChange(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
  Q_UNUSED (roles);
  changedFiles_ = getChangedFiles (topLeft.row (), bottomRight.row (), false);
}

void TestProject::handleDocumentsClose(const QModelIndex &parent, int start, int end)
{
  Q_UNUSED (parent);
  changedFiles_ = getChangedFiles (start, end, true); // Documents were modified before remove.
}

QStringList TestProject::getChangedFiles(int beginRow, int endRow, bool modifiedFlag) const
{
  QStringList files;
  for (int row = beginRow; row <= endRow; ++row)
  {
    DocumentModel::Entry* entry = DocumentModel::entryAtRow (row);
    if (entry == NULL)
    {
      continue;
    }
    IDocument* document = entry->document;
    if (document == NULL)
    {
      continue;
    }
    if (document->isModified () == modifiedFlag) // May not belong to project
    {
      files << document->filePath ().toString ();
    }
  }
  return files;
}

CustomRunConfiguration *TestProject::parse(Project *project)
{
  if (project == NULL)
  {
    return NULL;
  }

  //TODO build dependency table only on globalSnapshotChange signal?
  //TODO use snapshot instead of copying it to qhash?
  using namespace CppTools;
  Snapshot snapshot = CppModelManagerBase::instance ()->snapshot ();
  dependencyTable_.clear ();
  for (Snapshot::const_iterator i = snapshot.begin (), end = snapshot.end (); i != end; ++i)
  {
    const QString& fileName = i.key ().toString ();
    const Utils::FileNameList& dependingFiles = snapshot.filesDependingOn (fileName);
    dependencyTable_[fileName].clear ();
    for (Utils::FileNameList::const_iterator i = dependingFiles.begin (), end = dependingFiles.end (); i != end; ++i)
    {
      dependencyTable_[fileName].push_back (i->toString ());
    }
  }
  gtestIncludeFiles_ = gtestMainIncludes ();
  if (gtestIncludeFiles_.isEmpty ())
  {
    return NULL;
  }
  preprocessDependencyTable ();

  Target* target = project->activeTarget ();
  if (target == NULL)
  {
    return NULL;
  }

  LocalApplicationRunConfiguration* configuration =
      qobject_cast<LocalApplicationRunConfiguration*> (target->activeRunConfiguration ());
  if (configuration == NULL)
  {
    return NULL;
  }
  CustomRunConfiguration* config = new CustomRunConfiguration (configuration);
  return config;
}

QStringList TestProject::gtestMainIncludes() const
{
  QStringList gtestHeaders; // List because projects can have unique gtest.h includes.
  foreach (const QString& file, dependencyTable_.keys ())
  {
    if (file.endsWith (QLatin1Char('/') + gtestInclude)) // Should work fine because file contains full path
    {
      gtestHeaders << file;
    }
  }
  gtestHeaders.removeDuplicates();
  return gtestHeaders;
}

void TestProject::preprocessDependencyTable()
{
  QHash<QString, QStringList> newTable;
  for (QHash<QString, QStringList>::ConstIterator i = dependencyTable_.constBegin (),
       end = dependencyTable_.constEnd (); i != end; ++i)
  {
    QString newName = shortenFileName (i.key ());
    newTable [newName] += i.value ();
  }
  dependencyTable_ = newTable;
}
