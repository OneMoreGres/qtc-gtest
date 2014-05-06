#ifndef QTCGTEST_H
#define QTCGTEST_H

#include "PluginGlobal.h"

#include <extensionsystem/iplugin.h>

namespace QtcGtest {
  namespace Internal {

    class TestProject;

    /*!
     * \brief main plugin class.
     */
    class QtcGtestPlugin : public ExtensionSystem::IPlugin
    {
        Q_OBJECT
        Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QtCreatorPlugin" FILE "QtcGtest.json")

      public:
        QtcGtestPlugin();
        ~QtcGtestPlugin();

        bool initialize(const QStringList &arguments, QString *errorString);
        void extensionsInitialized();
        ShutdownFlag aboutToShutdown();

      private:
        void initLanguage ();
        void initMenus ();

      private:
        TestProject* testProject_;
    };

  } // namespace Internal
} // namespace QtcGtest

#endif // QTCGTEST_H

