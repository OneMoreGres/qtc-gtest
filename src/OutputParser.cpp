#include <QFileInfo>
#include <QRegularExpression>

#include "OutputParser.h"
#include "ParseState.h"
#include "TestModel.h"

using namespace QtcGtest::Internal;
namespace
{
  const QRegularExpression gtestStartPattern (
      QLatin1String ("^(.*)\\[==========\\] Running \\d+ tests? from \\d+ test cases?\\.\\s*$"));
  const QRegularExpression gtestEndPattern   (
      QLatin1String ("^(.*)\\[==========\\] (\\d+) tests? from (\\d+) test cases? ran. \\((\\d+) ms total\\)\\s*$"));

  const QRegularExpression newCasePattern    (
      QLatin1String ("^(.*)\\[\\-{10}\\] \\d+ tests? from (\\w+)\\s*$"));
  const QRegularExpression endCasePattern    (
      QLatin1String ("^(.*)\\[\\-{10}\\] \\d+ tests? from (\\w+) \\((\\d+) ms total\\)\\s*$"));

  const QRegularExpression beginTestPattern  (
      QLatin1String ("^(.*)\\[ RUN      \\] (\\w+)\\.(\\w+)\\s*$"));
  const QRegularExpression failTestPattern   (
      QLatin1String ("^(.*)\\[  FAILED  \\] (\\w+)\\.(\\w+) \\((\\d+) ms\\)\\s*$"));
  const QRegularExpression passTestPattern   (
      QLatin1String ("^(.*)\\[       OK \\] (\\w+)\\.(\\w+) \\((\\d+) ms\\)\\s*$"));
  const QRegularExpression failDetailPattern (
      QLatin1String ("^(.+):(\\d+): Failure\\s*$"));
}

OutputParser::OutputParser(QObject *parent) :
  QObject(parent)
{
}

bool OutputParser::isGoogleTestRun(const QString &line) const
{
  QRegularExpressionMatch match = gtestStartPattern.match (line);
  return (match.hasMatch ());
}

void OutputParser::parseMessage(const QString &line, TestModel &model, ParseState &state)
{
  QRegularExpressionMatch match;
  match = newCasePattern.match (line);
  if (match.hasMatch ())
  {
    state.currentCase = match.captured (2);
    state.passedCount = state.failedCount = 0;
    model.addCase (state.currentCase);
    return;
  }

  match = endCasePattern.match (line);
  if (match.hasMatch ())
  {
    int totalTime = match.captured (3).toInt ();
    model.updateCase (state.currentCase, state.passedCount, state.failedCount, totalTime);
    state.currentCase.clear ();
    state.currentTest.clear ();
    return;
  }

  match = beginTestPattern.match (line);
  if (match.hasMatch ())
  {
    state.currentTest = match.captured (3);
    model.addTest (state.currentTest, state.currentCase);
    return;
  }

  match = passTestPattern.match (line);
  if (match.hasMatch ())
  {
    QString unrelated = match.captured(1);
    if (!unrelated.isEmpty()) {
      model.addTestDetail (state.currentTest, state.currentCase, unrelated);
    }
    ++state.passedCount;
    ++state.passedTotalCount;
    int totalTime = match.captured (4).toInt ();
    model.updateTest (state.currentTest, state.currentCase, true, totalTime);
    state.currentTest.clear ();
    return;
  }

  match = failTestPattern.match (line);
  if (match.hasMatch ())
  {
    QString unrelated = match.captured(1);
    if (!unrelated.isEmpty()) {
      model.addTestDetail (state.currentTest, state.currentCase, unrelated);
    }
    ++state.failedCount;
    ++state.failedTotalCount;
    int totalTime = match.captured (4).toInt ();
    model.updateTest (state.currentTest, state.currentCase, false, totalTime);
    state.currentTest.clear ();
    return;
  }

  match = failDetailPattern.match (line);
  if (match.hasMatch ())
  {
    Q_ASSERT (!state.projectPath.isEmpty ());
    QString file = match.captured (2);
    QFileInfo info (file);
    if (info.isRelative ())
    {
      file = state.projectPath + QLatin1Char ('/') + match.captured (2);
    }
    int lineNumber = match.captured (3).toInt ();
    model.addTestError (state.currentTest, state.currentCase, line, file, lineNumber);
    return;
  }

  match = gtestEndPattern.match (line);
  if (match.hasMatch ())
  {
    state.totalTime = match.captured (3).toInt ();
    return;
  }

  if (!state.currentTest.isEmpty ())
  {
    Q_ASSERT (!state.currentCase.isEmpty ());
    model.addTestDetail (state.currentTest, state.currentCase, line);
  }
}
