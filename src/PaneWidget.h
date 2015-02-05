#ifndef PANEWIDGET_H
#define PANEWIDGET_H

#include <QWidget>
#include <QSortFilterProxyModel>

namespace Ui {
  class PaneWidget;
}

namespace QtcGtest {
  namespace Internal {
    class TestModel;

    class PaneWidget : public QWidget
    {
        Q_OBJECT

      public:
        explicit PaneWidget(TestModel* model, QWidget *parent = 0);
        ~PaneWidget();

      public:
        QModelIndex currentIndex () const;
        void setCurrentIndex (const QModelIndex& index);

      public slots:
        void showPassed (bool show);
        void spanColumns ();

      signals:
        void viewClicked (const QModelIndex& index);

      private:
        Ui::PaneWidget *ui;
        TestModel* model_;
        QSortFilterProxyModel* proxy_;
    };

  } // namespace Internal
} // namespace QtcGtest

#endif // PANEWIDGET_H
