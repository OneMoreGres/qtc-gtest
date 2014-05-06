#ifndef AUTOTOOLTIPDELEGATE_H
#define AUTOTOOLTIPDELEGATE_H

#include <QStyledItemDelegate>

namespace QtcGtest {
  namespace Internal {

    class AutoToolTipDelegate : public QStyledItemDelegate
    {
        Q_OBJECT
      public:
        AutoToolTipDelegate (QObject* parent);

      public slots:
        bool helpEvent (QHelpEvent* event, QAbstractItemView* view,
                        const QStyleOptionViewItem& option, const QModelIndex& index);
    };

  } // namespace Internal
} // namespace QtcGtest


#endif // AUTOTOOLTIPDELEGATE_H
