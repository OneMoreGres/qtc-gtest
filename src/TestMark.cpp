#include <utils/utilsicons.h>

#include "TestMark.h"
#include "Constants.h"
#include "OutputPane.h"

namespace QtcGtest {
  namespace Internal {

    TestMark::TestMark(QPersistentModelIndex index, const QString &fileName, int lineNumber,
                       OutputPane &pane)
      : TextEditor::TextMark (fileName, lineNumber, Core::Id (Constants::TEST_MARK_ID)),
        index_ (index), pane_(pane)
    {
      setVisible (true);
      setIcon(Utils::Icons::CRITICAL.icon());
      setPriority(TextEditor::TextMark::LowPriority);
    }

    bool TestMark::isClickable() const
    {
      return true;
    }

    void TestMark::clicked()
    {
      pane_.popup (OutputPane::NoModeSwitch);
      pane_.setCurrentIndex (index_);
    }
  }
}
