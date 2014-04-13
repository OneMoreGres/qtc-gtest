#include <QAction>
#include <QTranslator>

#include <coreplugin/icore.h>

#include <projectexplorer/projectexplorer.h>

#include <QtPlugin>

#include "QtcGtestPlugin.h"
#include "Constants.h"
#include "Settings.h"
#include "OutputPane.h"

using namespace QtcGtest::Internal;

using namespace Core;
using namespace ProjectExplorer;

QtcGtestPlugin::QtcGtestPlugin():
  IPlugin ()
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

  OutputPane* pane = new OutputPane;
  connect (ProjectExplorerPlugin::instance (), SIGNAL (runControlStarted(ProjectExplorer::RunControl *)),
           pane, SLOT (handleRunStart(ProjectExplorer::RunControl *)));
  connect (ProjectExplorerPlugin::instance (), SIGNAL (runControlFinished(ProjectExplorer::RunControl *)),
           pane, SLOT (handleRunFinish(ProjectExplorer::RunControl *)));
  addAutoReleasedObject (pane);

  return true;
}

void QtcGtestPlugin::initLanguage()
{
  const QString& language = Core::ICore::userInterfaceLanguage();
  if (!language.isEmpty())
  {
    QTranslator* translator = new QTranslator (this);
    const QString& creatorTrPath = ICore::resourcePath () + QLatin1String ("/translations");
    const QString& trFile = QLatin1String ("QtcGtest_") + language;
    if (translator->load (trFile, creatorTrPath))
    {
      qApp->installTranslator (translator);
    }
  }
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

Q_EXPORT_PLUGIN2(QtcGtest, QtcGtestPlugin)

