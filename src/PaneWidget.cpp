#include "PaneWidget.h"
#include "ui_PaneWidget.h"
#include "TestModel.h"
#include "AutoToolTipDelegate.h"

using namespace QtcGtest::Internal;

PaneWidget::PaneWidget(const QSharedPointer<TestModel> &model, QWidget *parent) :
  QWidget(parent),
  ui(new Ui::PaneWidget),
  model_ (model),
  proxy_ (new QSortFilterProxyModel (this))
{
  ui->setupUi(this);

  proxy_->setSourceModel (model.data());
  proxy_->setFilterKeyColumn (TestModel::ColumnFailed);

  connect (ui->caseView, SIGNAL (clicked (const QModelIndex&)),
           this, SIGNAL (viewClicked (const QModelIndex&)));

  ui->caseView->setModel (proxy_);
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

QModelIndex PaneWidget::testModelIndex(const QModelIndex &proxyIndex) const
{
  return proxy_->mapToSource(proxyIndex);
}

QModelIndex PaneWidget::proxyIndex(const QModelIndex &testModelIndex) const
{
  return proxy_->mapFromSource(testModelIndex);
}

void PaneWidget::showPassed(bool show)
{
  // Filter out 0 and empty ColumnFailed
  QString pattern = (show) ? QString () : QString (QLatin1String("-?[1-9]\\d*"));
  proxy_->setFilterRegExp (pattern);
  spanColumns ();
}

void PaneWidget::spanColumns()
{
  for (int i = 0, end = proxy_->rowCount(); i < end; ++i)
  {
    QModelIndex typeIndex = proxy_->index (i, TestModel::ColumnType);
    TestModel::Type caseType = TestModel::Type (typeIndex.data ().toInt ());
    if (caseType == TestModel::TypeNote)
    {
      ui->caseView->setFirstColumnSpanned (i, QModelIndex (), true);
      continue;
    }
    QModelIndex caseIndex = proxy_->index (i, TestModel::ColumnName);
    for (int ii = 0, iiEnd = proxy_->rowCount(caseIndex); ii < iiEnd; ++ii)
    {
      QModelIndex testIndex = proxy_->index (ii, TestModel::ColumnName, caseIndex);
      for (int iii = 0, iiiEnd = proxy_->rowCount(testIndex); iii < iiiEnd; ++iii)
      {
        ui->caseView->setFirstColumnSpanned (iii, testIndex, true);
      }
    }
  }
}
