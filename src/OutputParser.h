#ifndef OUTPUTPARSER_H
#define OUTPUTPARSER_H

#include <QObject>

namespace QtcGtest {
  namespace Internal {

    class TestModel;
    class ParseState;

    class OutputParser : public QObject
    {
        Q_OBJECT
      public:
        explicit OutputParser(QObject *parent = 0);

        bool isGoogleTestRun (const QString& line) const;

        void parseMessage (const QString& line, TestModel& model, ParseState& state);
    };

  } // namespace Internal
} // namespace QtcGtest

#endif // OUTPUTPARSER_H
