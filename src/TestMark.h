#ifndef TESTMARK_H
#define TESTMARK_H

#include <QPersistentModelIndex>

#include <texteditor/textmark.h>

namespace QtcGtest {
  namespace Internal {

    class OutputPane;

    class TestMark : public TextEditor::TextMark {
      public:
        TestMark (QPersistentModelIndex index, const QString &fileName, int lineNumber,
                  OutputPane &pane);

        bool isClickable () const override;
        void clicked () override;

      private:
        QPersistentModelIndex index_;
        OutputPane &pane_;
    };

  }
}

#endif // TESTMARK_H
