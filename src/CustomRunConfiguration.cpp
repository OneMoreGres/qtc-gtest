#include "CustomRunConfiguration.h"

#include <projectexplorer/environmentaspect.h>

using namespace ProjectExplorer;

QtcGtest::Internal::CustomRunConfiguration::CustomRunConfiguration(LocalApplicationRunConfiguration *source) :
  LocalApplicationRunConfiguration (source->target (), source)
{
  Q_ASSERT (source != NULL);
  executable_ = source->executable ();
  runMode_ = source->runMode ();
  workingDirectory_ = source->workingDirectory ();
  commandLineArguments_ = source->commandLineArguments ();
  EnvironmentAspect *aspect = source->extraAspect<EnvironmentAspect>();
  if (aspect != NULL) {
    environment_ = aspect->environment ();
  }
}

QWidget *QtcGtest::Internal::CustomRunConfiguration::createConfigurationWidget()
{
  return NULL;
}

QString QtcGtest::Internal::CustomRunConfiguration::executable() const
{
  return executable_;
}

ApplicationLauncher::Mode QtcGtest::Internal::CustomRunConfiguration::runMode() const
{
  return runMode_;
}

QString QtcGtest::Internal::CustomRunConfiguration::workingDirectory() const
{
  return workingDirectory_;
}

QString QtcGtest::Internal::CustomRunConfiguration::commandLineArguments() const
{
  return commandLineArguments_;
}
void QtcGtest::Internal::CustomRunConfiguration::addToBaseEnvironment(Utils::Environment &env) const
{
  env = environment_;
}

void QtcGtest::Internal::CustomRunConfiguration::setArguments(const QString &arguments)
{
  commandLineArguments_ = arguments;
}
