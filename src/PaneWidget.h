#ifndef PANEWIDGET_H
#define PANEWIDGET_H

#include <QWidget>
#include <QAbstractItemModel>

namespace Ui {
  class PaneWidget;
}

namespace QtcGtest {
  namespace Internal {

    class PaneWidget : public QWidget
    {
        Q_OBJECT

      public:
        explicit PaneWidget(QAbstractItemModel* model, QWidget *parent = 0);
        ~PaneWidget();

      public:
        QModelIndex currentIndex () const;
        void setCurrentIndex (const QModelIndex& index);

      signals:
        void viewClicked (const QModelIndex& index);

      private slots:
        void spanDetailRows(const QModelIndex &parent, int start, int end);

      private:
        Ui::PaneWidget *ui;
    };

  } // namespace Internal
} // namespace QtcGtest

#endif // PANEWIDGET_H
