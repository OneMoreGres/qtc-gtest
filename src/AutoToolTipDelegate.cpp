#include <QHelpEvent>
#include <QAbstractItemView>
#include <QToolTip>

#include "AutoToolTipDelegate.h"

QtcGtest::Internal::AutoToolTipDelegate::AutoToolTipDelegate(QObject *parent) :
  QStyledItemDelegate (parent)
{

}

bool QtcGtest::Internal::AutoToolTipDelegate::helpEvent(
    QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option,
    const QModelIndex &index)
{
  if (!event || !view)
    return false;

  if (event->type() == QEvent::ToolTip)
  {
    QRect rect = view->visualRect(index);
    QSize size = sizeHint(option, index);
    if (rect.width() < size.width())
    {
      QVariant tooltip = index.data(Qt::DisplayRole);
      if (tooltip.canConvert<QString>())
      {
        QToolTip::showText(event->globalPos(), QString(QLatin1String ("<div>%1</div>"))
                            .arg(tooltip.toString().toHtmlEscaped ()), view);
        return true;
      }
    }
    if (!QStyledItemDelegate::helpEvent(event, view, option, index))
    {
      QToolTip::hideText();
    }
    return true;
  }

  return QStyledItemDelegate::helpEvent(event, view, option, index);
}
