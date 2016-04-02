#ifndef PARSESTATE_H
#define PARSESTATE_H

#include <QStringList>

namespace QtcGtest {
  namespace Internal {

  class ParseState
  {
    public:
      ParseState ();
      void reset ();

      bool isGoogleTestRun;
      QStringList projectFiles;
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
