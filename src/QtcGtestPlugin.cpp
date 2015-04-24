#include <QAction>
#include <QTranslator>
#include <QMenu>
#include <QCoreApplication>
#include <QAbstractItemModel>

#include <coreplugin/icore.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/editormanager/editormanager.h>
#include <projectexplorer/projectexplorer.h>

#include <QtPlugin>

#include "QtcGtestPlugin.h"
#include "Constants.h"
#include "Settings.h"
#include "OutputPane.h"
#include "TestProject.h"

using namespace QtcGtest::Internal;

using namespace Core;
using namespace ProjectExplorer;

QtcGtestPlugin::QtcGtestPlugin():
  IPlugin (),
  testProject_ (new TestProject (this))
{
  // Create your members
}

QtcGtestPlugin::~QtcGtestPlugin()
{
  // Unregister objects from the plugin manager's object pool
  // Delete members
}

bool QtcGtestPlugin::initialize(const QStringList &arguments, QString *errorString)
{
  // Register objects in the plugin manager's object pool
  // Load settings
  // Add actions to menus
  // Connect to other plugins' signals
  // In the initialize function, a plugin can be sure that the plugins it
  // depends on have initialized their members.

  Q_UNUSED(arguments)
  Q_UNUSED(errorString)

  initLanguage ();
  initMenus ();

  OutputPane* pane = new OutputPane;
  connect (ProjectExplorerPlugin::instance (), SIGNAL (runControlStarted(ProjectExplorer::RunControl *)),
           pane, SLOT (handleRunStart(ProjectExplorer::RunControl *)));
  connect (ProjectExplorerPlugin::instance (), SIGNAL (runControlFinished(ProjectExplorer::RunControl *)),
           pane, SLOT (handleRunFinish(ProjectExplorer::RunControl *)));
  addAutoReleasedObject (pane);


  connect (DocumentModel::model (),
           SIGNAL (dataChanged(const QModelIndex &, const QModelIndex &, const QVector<int> &)),
           testProject_,
           SLOT (handleDocumentsChange(const QModelIndex &, const QModelIndex &, const QVector<int> &)));
  connect (DocumentModel::model (),
           SIGNAL (rowsAboutToBeRemoved(const QModelIndex &, int, int)),
           testProject_,
           SLOT (handleDocumentsClose(const QModelIndex &, int, int)));

  return true;
}

void QtcGtestPlugin::initLanguage()
{
  const QString& language = Core::ICore::userInterfaceLanguage();
  if (!language.isEmpty())
  {
    QStringList paths;
    paths << ICore::resourcePath () << ICore::userResourcePath();
    const QString& trFile = QLatin1String ("QtcGtest_") + language;
    QTranslator* translator = new QTranslator (this);
    foreach (const QString& path, paths)
    {
      if (translator->load (trFile, path + QLatin1String ("/translations")))
      {
        qApp->installTranslator (translator);
        break;
      }
    }
  }
}

void QtcGtestPlugin::initMenus()
{
  QAction *checkProjectAction = new QAction(tr("Check project"), this);
  Core::Command *checkProjectCmd = ActionManager::registerAction(
                                     checkProjectAction, Constants::ACTION_CHECK_PROJECT_ID,
                                     Context(Core::Constants::C_GLOBAL));
  checkProjectCmd->setDefaultKeySequence (QKeySequence (tr ("Alt+T,Alt+A")));
  connect(checkProjectAction, SIGNAL(triggered()), testProject_, SLOT(checkProject()));

  QAction *checkCurrentAction = new QAction(tr("Check current"), this);
  Core::Command *checkCurrentCmd = ActionManager::registerAction(
                                     checkCurrentAction, Constants::ACTION_CHECK_CURRENT_ID,
                                     Context(Core::Constants::C_GLOBAL));
  checkCurrentCmd->setDefaultKeySequence (QKeySequence (tr ("Alt+T,Alt+C")));
  connect(checkCurrentAction, SIGNAL(triggered()), testProject_, SLOT(checkCurrent()));

  QAction *checkChangedAction = new QAction(tr("Check changed"), this);
  Core::Command *checkChangedCmd = ActionManager::registerAction(
                                     checkChangedAction, Constants::ACTION_CHECK_CHANGED_ID,
                                     Context(Core::Constants::C_GLOBAL));
  checkChangedCmd->setDefaultKeySequence (QKeySequence (tr ("Alt+T,Alt+T")));
  connect(checkChangedAction, SIGNAL(triggered()), testProject_, SLOT(checkChanged()));

  ActionContainer *menu = ActionManager::createMenu(Constants::MENU_ID);
  menu->menu()->setTitle(tr("Google Test"));
  menu->addAction(checkProjectCmd);
  menu->addAction(checkCurrentCmd);
  menu->addAction(checkChangedCmd);
  ActionManager::actionContainer(Core::Constants::M_TOOLS)->addMenu(menu);
}

void QtcGtestPlugin::extensionsInitialized()
{
  // Retrieve objects from the plugin manager's object pool
  // In the extensionsInitialized function, a plugin can be sure that all
  // plugins that depend on it are completely initialized.
}

ExtensionSystem::IPlugin::ShutdownFlag QtcGtestPlugin::aboutToShutdown()
{
  // Save settings
  // Disconnect from signals that are not needed during shutdown
  // Hide UI (if you add UI that is not in the main window directly)
  return SynchronousShutdown;
}
