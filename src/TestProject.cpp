#include <QDebug>
#include <QFileInfo>
#include <QRegularExpression>
#include <QFormLayout>

#include <cpptools/cppmodelmanager.h>
#include <cpptools/searchsymbols.h>
#include <cplusplus/DependencyTable.h>
#include <projectexplorer/project.h>
#include <projectexplorer/session.h>
#include <projectexplorer/projectexplorer.h>
#include <projectexplorer/target.h>
#include <projectexplorer/runconfigurationaspects.h>
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/idocument.h>
#include <extensionsystem/pluginmanager.h>

#include "TestProject.h"

using namespace QtcGtest::Internal;

using namespace Core;
using namespace ProjectExplorer;
using namespace CppTools;
using namespace CPlusPlus;
using namespace Utils;

namespace {
  const QString gtestInclude = QLatin1String ("gtest.h");
  const QString gtestFilter = QLatin1String ("--gtest_filter=%1");
  const QString gtestFilterSeparator = QLatin1String (":");
  const QRegularExpression testPattern (
    QLatin1String ("(TEST|TEST_F|TYPED_TEST|TYPED_TEST_P|TEST_P)\\s*\\((\\w+),\\s*\\w+\\)"));
  enum Test {TestType = 1, TestCase, TestName};


  FileName shortenFileName (const FileName &file) {
    QFileInfo f (file.toString ());
    FileName newName = FileName::fromString (f.absolutePath () + QLatin1Char ('/') + f.baseName ());
    return newName;
  }

  template <class T>
  void removeDuplicates (T &list) {
    auto last = std::unique (list.begin (), list.end ());
    list.erase (last, list.end ());
  }
}

TestProject::TestProject (QObject *parent) :
  QObject (parent) {
  testFilterPatterns_[QLatin1String ("TYPED_TEST")] = QLatin1String ("%1/*.*");
  testFilterPatterns_[QLatin1String ("TYPED_TEST_P")] = QLatin1String ("*/%1/*.*");
  testFilterPatterns_[QLatin1String ("TEST_P")] = QLatin1String ("*/%1.*");
  testFilterPatterns_[QLatin1String ("TEST")] = QLatin1String ("%1.*");
  testFilterPatterns_[QLatin1String ("TEST_F")] = QLatin1String ("%1.*");
}

void TestProject::checkProject () {
  Project *project = SessionManager::startupProject ();
  RunConfiguration *configuration = parse (project);
  if (configuration != NULL) {
    runTests (configuration);
  }
}

void TestProject::runTests (RunConfiguration *configuration) {
  Q_ASSERT (configuration != NULL);
  ProjectExplorerPlugin *plugin = ProjectExplorerPlugin::instance ();
  auto runControl = new ProjectExplorer::RunControl (
    configuration, ProjectExplorer::Constants::NORMAL_RUN_MODE);

  auto producer = RunControl::producer (
    configuration, ProjectExplorer::Constants::NORMAL_RUN_MODE);
  QTC_ASSERT (producer, return );

  if (!runControl) {
    qDebug () << "failed to create run control";
    return;
  }
  emit runControlAboutToStart (runControl);
  (void) producer (runControl);

  plugin->startRunControl (runControl);
}

void TestProject::checkChanged () {
  if (changedFiles_.isEmpty ()) {
    return;
  }
  Project *project = SessionManager::startupProject ();
  RunConfiguration *configuration = parse (project);
  if (configuration != NULL) {
    runTestsForFiles (changedFiles_, configuration);
    changedFiles_.clear ();
  }
}

void TestProject::checkCurrent () {
  Project *project = SessionManager::startupProject ();
  RunConfiguration *configuration = parse (project);
  if (configuration == NULL) {
    return;
  }

  IDocument *document = EditorManager::currentDocument ();
  if (document == NULL) {
    return;
  }
  FileName file = document->filePath ();
  QStringList files = project->files (Project::SourceFiles);
  if (!files.contains (file.toString ())) {
    return;
  }

  runTestsForFiles (FileNameList () << file, configuration);
}

void TestProject::runTestsForFiles (const FileNameList &files, RunConfiguration *configuration) {
  Q_ASSERT (configuration != NULL);
  Q_ASSERT (!gtestIncludeFiles_.isEmpty ());
  QSet<FileName> testFiles = getDependentFiles (gtestIncludeFiles_).toSet ();
  QSet<FileName> dependentFiles = getDependentFiles (files).toSet ();
  testFiles.intersect (dependentFiles);
  if (testFiles.isEmpty ()) {
    return;
  }

  QStringList testCases = getTestCases (testFiles);
  if (testCases.isEmpty ()) {
    return;
  }
  QString arguments = gtestFilter.arg (testCases.join (gtestFilterSeparator));
  ArgumentsAspect *aspect = configuration->extraAspect <ArgumentsAspect> ();
  if (aspect != NULL) {
    // hack to avoid segv inside aspect. not so terrible because this aspect is for 1 use only
    QWidget fakeWidget;
    QFormLayout fakeLayout;
    aspect->addToMainConfigurationWidget (&fakeWidget, &fakeLayout);
    aspect->setArguments (aspect->unexpandedArguments () + arguments);
  }
  runTests (configuration);
}

FileNameList TestProject::getDependentFiles (const FileNameList &files) const {
  FileNameList dependentFiles;
  FileNameList uncheckedFiles = files;
  while (!uncheckedFiles.isEmpty ()) {
    FileName file = uncheckedFiles.takeFirst ();
    if (dependentFiles.contains (file)) {
      continue;
    }
    dependentFiles << file;
    FileName newName = shortenFileName (file);
    FileNameList newFiles = dependencyTable_.value (newName);
    if (!newFiles.isEmpty ()) {
      dependentFiles += newFiles;
      uncheckedFiles += newFiles;
    }
  }
  removeDuplicates (dependentFiles);
  return dependentFiles;
}

QStringList TestProject::getTestCases (const QSet<FileName> &fileNames) const {
  QStringList testCases;
  foreach (const FileName &file, fileNames) {
    QFile f (file.toString ());
    if (!f.open (QFile::ReadOnly)) {
      continue;
    }
    //TODO Support codecs?
    QString source = QString::fromLocal8Bit (f.readAll ());
    f.close ();
    QRegularExpressionMatchIterator i = testPattern.globalMatch (source);
    while (i.hasNext ()) {
      QRegularExpressionMatch match = i.next ();
      QString caseType = match.captured (TestType);
      Q_ASSERT (testFilterPatterns_.contains (caseType));
      QString pattern = testFilterPatterns_[caseType].arg (match.captured (TestCase));
      testCases << pattern;
    }
  }
  testCases.removeDuplicates ();
  return testCases;
}

void TestProject::handleDocumentsChange (const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles) {
  Q_UNUSED (roles);
  changedFiles_ = getChangedFiles (topLeft.row (), bottomRight.row (), false);
}

void TestProject::handleDocumentsClose (const QModelIndex &parent, int start, int end) {
  Q_UNUSED (parent);
  changedFiles_ = getChangedFiles (start, end, true); // Documents were modified before remove.
}

FileNameList TestProject::getChangedFiles (int beginRow, int endRow, bool modifiedFlag) const {
  FileNameList files;
  for (int row = beginRow; row <= endRow; ++row) {
    DocumentModel::Entry *entry = DocumentModel::entryAtRow (row);
    if (entry == NULL) {
      continue;
    }
    IDocument *document = entry->document;
    if (document == NULL) {
      continue;
    }
    if (document->isModified () == modifiedFlag) { // May not belong to project
      files << document->filePath ();
    }
  }
  return files;
}

RunConfiguration *TestProject::parse (Project *project) {
  if (project == NULL) {
    return NULL;
  }

  //TODO build dependency table only on globalSnapshotChange signal?
  //TODO use snapshot instead of copying it to qhash?
  using namespace CppTools;
  Snapshot snapshot = CppModelManagerBase::instance ()->snapshot ();
  dependencyTable_.clear ();
  for (Snapshot::const_iterator i = snapshot.begin (), end = snapshot.end (); i != end; ++i) {
    const FileName &fileName = i.key ();
    dependencyTable_[fileName] = snapshot.filesDependingOn (fileName);
  }
  gtestIncludeFiles_ = gtestMainIncludes ();
  if (gtestIncludeFiles_.isEmpty ()) {
    return NULL;
  }
  preprocessDependencyTable ();

  Target *target = project->activeTarget ();
  if (target == NULL) {
    return NULL;
  }

  RunConfiguration *configuration =
    qobject_cast<RunConfiguration *> (target->activeRunConfiguration ());

  Target *parent = qobject_cast<Target *>(configuration->parent ());
  if (parent) {
    IRunConfigurationFactory *factory =
      ExtensionSystem::PluginManager::getObject<IRunConfigurationFactory>(
        [configuration, parent](IRunConfigurationFactory *factory) {
      return factory->canClone (parent, configuration);
    });
    if (factory) {
      return factory->clone (parent, configuration);
    }
  }
  return NULL;
}

FileNameList TestProject::gtestMainIncludes () const {
  FileNameList gtestHeaders; // List because projects can have unique gtest.h includes.
  foreach (const FileName &file, dependencyTable_.keys ()) {
    if (file.endsWith (QLatin1Char ('/') + gtestInclude)) { // Should work fine because file contains full path
      gtestHeaders << file;
    }
  }
  removeDuplicates (gtestHeaders);
  return gtestHeaders;
}

void TestProject::preprocessDependencyTable () {
  QHash<FileName, FileNameList> newTable;
  for (QHash<FileName, FileNameList>::ConstIterator i = dependencyTable_.constBegin (),
       end = dependencyTable_.constEnd (); i != end; ++i) {
    FileName newName = shortenFileName (i.key ());
    newTable [newName] += i.value ();
  }
  dependencyTable_ = newTable;
}
