#ifndef PARSESTATE_H
#define PARSESTATE_H

#include <QString>

namespace QtcGtest {
  namespace Internal {

  class ParseState
  {
    public:
      ParseState ();
      void reset ();

      bool isGoogleTestRun;
      QString projectPath;
      QString currentCase;
      QString currentTest;
      int passedCount;
      int failedCount;
      int passedTotalCount;
      int failedTotalCount;
      int totalTime;
      int disabledCount;
  };

} // namespace Internal
} // namespace QtcGtest


#endif // PARSESTATE_H
