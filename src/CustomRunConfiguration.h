#ifndef CUSTOMRUNCONFIGURATION_H
#define CUSTOMRUNCONFIGURATION_H

#include <projectexplorer/localapplicationrunconfiguration.h>
#include <utils/environment.h>

namespace ProjectExplorer
{
  class Target;
}

namespace QtcGtest {
  namespace Internal {

    class CustomRunConfiguration : public ProjectExplorer::LocalApplicationRunConfiguration
    {
        Q_OBJECT
      public:
        explicit CustomRunConfiguration(ProjectExplorer::LocalApplicationRunConfiguration* source);

        QWidget *createConfigurationWidget();
        QString executable() const;
        ProjectExplorer::ApplicationLauncher::Mode runMode() const;
        QString workingDirectory() const;
        QString commandLineArguments() const;
        void addToBaseEnvironment(Utils::Environment &env) const;

        void setArguments (const QString& arguments);

      private:
        QString executable_;
        ProjectExplorer::ApplicationLauncher::Mode runMode_;
        QString workingDirectory_;
        QString commandLineArguments_;
        Utils::Environment environment_;
    };
  } // namespace Internal
} // namespace QtcGtest


#endif // CUSTOMRUNCONFIGURATION_H
