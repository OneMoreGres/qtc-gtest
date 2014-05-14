#include <QDebug>
#include <QFileInfo>
#include <QRegularExpression>

#include <cpptools/cppmodelmanager.h>
#include <cpptools/searchsymbols.h>
#include <cplusplus/DependencyTable.h>
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
  const QRegularExpression testPattern (QLatin1String ("(?:TEST|TEST_F)\\s*\\((\\w+),\\s*\\w+\\)"));


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
  QString file = document->filePath ();
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
  Q_ASSERT (gtestIncludeFile_.isEmpty ());
  QString gtestFile = shortenFileName (gtestIncludeFile_);
  QSet<QString> testFiles = dependencyTable_.value (gtestFile).toSet ();

  QStringList dependentFiles = getDependentFiles (files);
  testFiles.intersect (dependentFiles.toSet ());
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
      testCases << (match.captured (1) + QLatin1String (".*"));
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
  DocumentModel* model = EditorManager::documentModel ();
  Q_ASSERT (model != NULL);
  QStringList files;
  for (int row = beginRow; row <= endRow; ++row)
  {
    DocumentModel::Entry* entry = model->documentAtRow (row);
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
      files << document->filePath ();
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

  //Internal? How to get this properly?
  CppTools::Internal::CppModelManager *modelManager =
      CppTools::Internal::CppModelManager::instance();

  DependencyTable table;
  table.build (modelManager->snapshot ());
  dependencyTable_ = table.dependencyTable ();
  gtestIncludeFile_ = gtestMainInclude ();
  if (gtestIncludeFile_.isEmpty ())
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

QString TestProject::gtestMainInclude() const
{
  foreach (const QString& file, dependencyTable_.keys ())
  {
    if (file.endsWith (gtestInclude))
    {
      return file;
    }
  }
  return QString ();
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
