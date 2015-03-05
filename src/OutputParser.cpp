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
  enum GtestStart {GtestStartUnrelated = 1};
  const QRegularExpression gtestEndPattern   (
      QLatin1String ("^(.*)\\[==========\\] (\\d+) tests? from (\\d+) test cases? ran. \\((\\d+) ms total\\)\\s*$"));
  enum GtestEnd{GtestEndUnrelated = 1, GtestEndTestsRun, GtestEndCasesRun, GtestEndTimeSpent};
  const QRegularExpression gtestDisabledPattern (
      QLatin1String ("^\\s*YOU HAVE (\\d+) DISABLED TESTS?\\s*$"));
  enum GtestDisabled{GtestDisabledCount = 1};
  const QRegularExpression gtestFilterPattern (
      QLatin1String ("^\\s*Note: (Google Test filter = .*)\\s*$"));
  enum GtestFilter{GtestFilterLine = 1};

  const QRegularExpression newCasePattern    (
      QLatin1String ("^(.*)\\[\\-{10}\\] \\d+ tests? from ([\\w/]+)(, where TypeParam = (.+))?\\s*$"));
  enum NewCase{NewCaseUnrelated = 1, NewCaseName, NewCaseFullParameter, NewCaseParameterType};
  const QRegularExpression endCasePattern    (
      QLatin1String ("^(.*)\\[\\-{10}\\] \\d+ tests? from ([\\w/]+) \\((\\d+) ms total\\)\\s*$"));
  enum EndCase{EndCaseUnrelated = 1, EndCaseName, EndCaseTimeSpent};

  const QRegularExpression beginTestPattern  (
      QLatin1String ("^(.*)\\[ RUN      \\] ([\\w/]+)\\.([\\w/]+)\\s*$"));
  enum NewTest{NewTestUnrelated = 1, NewTestCaseName, NewTestName};
  const QRegularExpression failTestPattern   (
      QLatin1String ("^(.*)\\[  FAILED  \\] ([\\w/]+)\\.([\\w/]+)(, where (GetParam\\(\\)|TypeParam) = (.+))? \\((\\d+) ms\\)\\s*$"));
  enum FailTest{FailTestUnrelated = 1, FailTestCaseName, FailTestName, FailTestFullParameter,
                FailTestParameterType, FailTestParameterDetail, FailTestTimeSpent};
  const QRegularExpression passTestPattern   (
      QLatin1String ("^(.*)\\[       OK \\] ([\\w/]+)\\.([\\w/]+) \\((\\d+) ms\\)\\s*$"));
  enum PassTest{PassTestUnrelated = 1, PassTestCaseName, PassTestName, PassTestTimeSpent};
  const QRegularExpression failDetailPattern (
      QLatin1String ("^(.+)[\\(:](\\d+)\\)?: (?:Failure|error).*$"));
  enum FailDetail{FailDetailFileName = 1, FailDetailLine};
}

OutputParser::OutputParser(QObject *parent) :
  QObject(parent)
{
}

bool OutputParser::isGoogleTestRun(const QString &line) const
{
  QRegularExpressionMatch match = gtestStartPattern.match (line);
  QRegularExpressionMatch matchFilter = gtestFilterPattern.match (line);
  return (match.hasMatch () || matchFilter.hasMatch ());
}

void OutputParser::parseMessage(const QString &line, TestModel &model, ParseState &state)
{
  QRegularExpressionMatch match;
  match = newCasePattern.match (line);
  if (match.hasMatch ())
  {
    state.currentCase = match.captured (NewCaseName);
    if (match.lastCapturedIndex() == NewCaseParameterType)
    {
      state.currentCase += QString (QLatin1String(" <%1>")).arg (match.captured (NewCaseParameterType));
    }
    state.passedCount = state.failedCount = 0;
    model.addCase (state.currentCase);
    return;
  }

  match = endCasePattern.match (line);
  if (match.hasMatch ())
  {
    int totalTime = match.captured (EndCaseTimeSpent).toInt ();
    model.updateCase (state.currentCase, state.passedCount, state.failedCount, totalTime);
    state.currentCase.clear ();
    state.currentTest.clear ();
    return;
  }

  match = beginTestPattern.match (line);
  if (match.hasMatch ())
  {
    state.currentTest = match.captured (NewTestName);
    model.addTest (state.currentTest, state.currentCase);
    return;
  }

  match = passTestPattern.match (line);
  if (match.hasMatch ())
  {
    QString unrelated = match.captured(PassTestUnrelated);
    if (!unrelated.isEmpty()) {
      model.addTestDetail (state.currentTest, state.currentCase, unrelated);
    }
    ++state.passedCount;
    ++state.passedTotalCount;
    int totalTime = match.captured (PassTestTimeSpent).toInt ();
    model.updateTest (state.currentTest, state.currentCase, true, totalTime);
    state.currentTest.clear ();
    return;
  }

  match = failTestPattern.match (line);
  if (match.hasMatch ())
  {
    QString unrelated = match.captured(PassTestUnrelated);
    if (!unrelated.isEmpty()) {
      model.addTestDetail (state.currentTest, state.currentCase, unrelated);
    }
    ++state.failedCount;
    ++state.failedTotalCount;
    int totalTime = match.captured (FailTestTimeSpent).toInt ();
    model.updateTest (state.currentTest, state.currentCase, false, totalTime);
    QString parameterDetail = match.captured(FailTestParameterDetail);
    if (!parameterDetail.isEmpty()) {
      QString newTestName = state.currentTest + QString (QLatin1String(" <%1>")).arg (parameterDetail);
      model.renameTest (state.currentTest, newTestName, state.currentCase);
    }
    state.currentTest.clear ();
    return;
  }

  match = failDetailPattern.match (line);
  if (match.hasMatch ())
  {
    Q_ASSERT (!state.projectPath.isEmpty ());
    QString file = match.captured (FailDetailFileName);
    QFileInfo info (file);
    if (info.isRelative ())
    {
      file = state.projectPath + QLatin1Char ('/') + match.captured (FailDetailFileName);
    }
    int lineNumber = match.captured (FailDetailLine).toInt ();
    model.addTestError (state.currentTest, state.currentCase, line, file, lineNumber);
    return;
  }

  match = gtestEndPattern.match (line);
  if (match.hasMatch ())
  {
    state.totalTime = match.captured (GtestEndTimeSpent).toInt ();
    return;
  }

  match = gtestDisabledPattern.match (line);
  if (match.hasMatch ())
  {
    state.disabledCount = match.captured (GtestDisabledCount).toInt ();
    return;
  }

  match = gtestFilterPattern.match (line);
  if (match.hasMatch ())
  {
    model.addNote(match.captured (GtestFilterLine));
    return;
  }

  if (!state.currentTest.isEmpty ())
  {
    Q_ASSERT (!state.currentCase.isEmpty ());
    model.addTestDetail (state.currentTest, state.currentCase, line);
  }
}
