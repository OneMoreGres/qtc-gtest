#include "ParseState.h"

using namespace QtcGtest::Internal;

ParseState::ParseState () {
  reset ();
}

void ParseState::reset () {
  isGoogleTestRun = false;
  currentCase = currentTest = QString ();
  passedCount = passedTotalCount = failedCount = failedTotalCount = totalTime = disabledCount = 0;
  projectFiles.clear ();
}
