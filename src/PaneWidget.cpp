#include "PaneWidget.h"
#include "ui_PaneWidget.h"
#include "TestModel.h"
#include "AutoToolTipDelegate.h"

using namespace QtcGtest::Internal;

PaneWidget::PaneWidget(QAbstractItemModel *model, QWidget *parent) :
  QWidget(parent),
  ui(new Ui::PaneWidget)
{
  Q_ASSERT (model != NULL);
  ui->setupUi(this);

  connect (model, SIGNAL (rowsInserted(const QModelIndex &, int, int)),
           this, SLOT (spanDetailRows(const QModelIndex &, int, int)));
  connect (ui->caseView, SIGNAL (clicked (const QModelIndex&)),
           this, SIGNAL (viewClicked (const QModelIndex&)));

  ui->caseView->setModel (model);
  ui->caseView->hideColumn (TestModel::ColumnType);
  ui->caseView->header ()->setSectionResizeMode(QHeaderView::ResizeToContents);
  ui->caseView->setItemDelegate (new AutoToolTipDelegate (ui->caseView));
}

PaneWidget::~PaneWidget()
{
  delete ui;
}

QModelIndex PaneWidget::currentIndex() const
{
  return ui->caseView->currentIndex ();
}

void PaneWidget::setCurrentIndex(const QModelIndex &index)
{
  ui->caseView->setCurrentIndex (index);
}

void PaneWidget::spanDetailRows(const QModelIndex &parent, int start, int end)
{
  TestModel* model = qobject_cast<TestModel*> (ui->caseView->model());
  for (int i = start; i <= end; ++i)
  {
    QModelIndex added = model->index (start, TestModel::ColumnName, parent);
    Q_ASSERT (added.isValid ());
    TestModel::Type type = model->getType (added);
    if (type == TestModel::TypeDetail ||
        type == TestModel::TypeDetailError ||
        type == TestModel::TypeNote)
    {
      ui->caseView->setFirstColumnSpanned (i, parent, true);
    }
  }
}
