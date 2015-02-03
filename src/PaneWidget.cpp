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
  if (!parent.isValid()) {
    return;
  }
  const TestModel* model = qobject_cast<const TestModel*> (parent.model ());
  Q_ASSERT (model != NULL);
  if (model->getType (parent) != TestModel::TypeTest)
  {
    return;
  }
  for (int i = start; i <= end; ++i)
  {
    ui->caseView->setFirstColumnSpanned (i, parent, true);
  }
}
