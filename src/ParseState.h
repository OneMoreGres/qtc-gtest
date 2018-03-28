#ifndef PARSESTATE_H
#define PARSESTATE_H

#include <utils/fileutils.h>

namespace QtcGtest {
  namespace Internal {

    class ParseState {
      public:
        ParseState ();
        void reset ();

        bool isGoogleTestRun;
        Utils::FileNameList projectFiles;
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
