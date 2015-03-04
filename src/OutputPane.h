#ifndef OUTPUTPANE_H
#define OUTPUTPANE_H

#include <QLabel>
#include <QPointer>
#include <QToolButton>

#include <coreplugin/ioutputpane.h>
#include <utils/outputformat.h>

namespace ProjectExplorer
{
  class RunControl;
}

namespace QtcGtest {
  namespace Internal {

    class PaneWidget;
    class OutputParser;
    class TestModel;
    class ParseState;

    /*!
   * \brief Output pane for google test control.
   */
    class OutputPane : public Core::IOutputPane
    {
        Q_OBJECT
      public:
        explicit OutputPane(QObject *parent = 0);
        ~OutputPane ();

        // IOutputPane interface
      public:
        QWidget *outputWidget(QWidget *parent);
        QList<QWidget *> toolBarWidgets() const;
        QString displayName() const;
        int priorityInStatusBar() const;
        void clearContents();
        void visibilityChanged(bool visible);
        void setFocus();
        bool hasFocus() const;
        bool canFocus() const;
        bool canNavigate() const;
        bool canNext() const;
        bool canPrevious() const;
        void goToNext();
        void goToPrev();

      public slots:
        void handleRunStart (ProjectExplorer::RunControl* control);
        void handleRunFinish (ProjectExplorer::RunControl* control);

      private slots:
        void parseMessage (ProjectExplorer::RunControl *control,
                           const QString &msg, Utils::OutputFormat format);
        void handleViewClicked (const QModelIndex &index);

      private:
        void showError (const QModelIndex& errorIndex);

      private:
        OutputParser* parser_;
        QSharedPointer<TestModel> model_;
        ParseState* state_;

        // output widget
        QPointer<PaneWidget> widget_;

        // toolBarWidgets
        QLabel* totalsLabel_;
        QLabel* disabledLabel_;
        QToolButton* togglePopupButton_;
        QToolButton* togglePassedButton_;
    };

  } // namespace Internal
} // namespace QtcGtest

#endif // OUTPUTPANE_H
