#include "TestModel.h"

using namespace QtcGtest::Internal;

namespace
{
  const QColor goodColor = QColor ("#33CC66");
  const QColor badColor = QColor ("#FF6666");
  const QColor noteColor = QColor ("#F0E68C");
}

TestModel::TestModel(QObject *parent) :
  QStandardItemModel(parent), errorCount_ (0)
{
  columnNames_.insert (int (ColumnName), tr ("Name"));
  columnNames_.insert (int (ColumnPassed), tr ("Passed"));
  columnNames_.insert (int (ColumnFailed), tr ("Failed"));
  columnNames_.insert (int (ColumnTime), tr ("Time"));
  columnNames_.insert (int (ColumnType), tr ("Type"));
  setColumnCount (ColumnCount);
  setHeaderData (int (ColumnName), Qt::Horizontal, tr ("Name"));
  setHeaderData (int (ColumnPassed), Qt::Horizontal, tr ("Passed"));
  setHeaderData (int (ColumnFailed), Qt::Horizontal, tr ("Failed"));
  setHeaderData (int (ColumnTime), Qt::Horizontal, tr ("Time"));
  setHeaderData (int (ColumnType), Qt::Horizontal, tr ("Type"));
}

QModelIndex TestModel::findItem(const QString &name, const QModelIndex &parent) const
{
  Q_ASSERT (!name.isEmpty ());
  for (int i = 0, end = rowCount (parent); i < end; ++i)
  {
    QModelIndex item = index (i, ColumnName, parent);
    if (item.data () == name)
    {
      return item;
    }
  }
  return QModelIndex ();
}

QModelIndex TestModel::caseIndex(const QString &name) const
{
  Q_ASSERT (!name.isEmpty ());
  return findItem (name, QModelIndex ());
}

QModelIndex TestModel::testIndex(const QString &name,
                                  const QString &caseName) const
{
  Q_ASSERT (!name.isEmpty ());
  QModelIndex caseIndex = this->caseIndex (caseName);
  Q_ASSERT (caseIndex.isValid ());
  return findItem (name, caseIndex);
}

QModelIndex TestModel::previousError(const QModelIndex &index) const
{
  int currentCase, currentTest, currentDetail;
  getCurrentRows (index, currentCase, currentTest, currentDetail);
  if (currentCase == 0 && currentTest == 0 && currentDetail == 0)
  {
    return QModelIndex ();
  }
  --currentDetail;
  if (currentDetail == -1)
  {
    --currentTest;
  }
  if (currentTest == -1)
  {
    --currentCase;
  }

  for (; currentCase >= 0; --currentCase)
  {
    QModelIndex caseIndex = this->index (currentCase, 0);
    if (currentTest == -1)
    {
      currentTest = rowCount (caseIndex) - 1;
    }
    for (; currentTest >= 0; --currentTest)
    {
      QModelIndex testIndex = this->index (currentTest, 0, caseIndex);
      if (currentDetail == -1)
      {
        currentDetail = rowCount (testIndex) - 1;
      }
      for (; currentDetail >= 0; --currentDetail)
      {
        QModelIndex detailIndex = this->index (currentDetail, 0, testIndex);
        if (getType (detailIndex) == TypeDetailError)
        {
          return detailIndex;
        }
      }
    }
  }
  return QModelIndex ();
}

QModelIndex TestModel::nextError(const QModelIndex &index) const
{
  int currentCase, currentTest, currentDetail;
  Type type = getCurrentRows (index, currentCase, currentTest, currentDetail);
  if (type == TypeDetail || type == TypeDetailError)
  {
    ++currentDetail;
  }

  for (int iEnd = rowCount (); currentCase < iEnd; ++currentCase)
  {
    QModelIndex caseIndex = this->index (currentCase, 0);
    for (int iiEnd = rowCount (caseIndex); currentTest < iiEnd; ++currentTest)
    {
      QModelIndex testIndex = this->index (currentTest, 0, caseIndex);
      for (int iiiEnd = rowCount (testIndex); currentDetail < iiiEnd; ++currentDetail)
      {
        QModelIndex detailIndex = this->index (currentDetail, 0, testIndex);
        if (getType (detailIndex) == TypeDetailError)
        {
          return detailIndex;
        }
      }
      currentDetail = 0;
    }
    currentTest = 0;
  }
  return QModelIndex ();
}

TestModel::Type TestModel::getCurrentRows(const QModelIndex &index, int &caseRow, int &testRow, int &detailRow) const
{
  caseRow = testRow = detailRow = 0;
  Type type = getType (index);
  if (type == TypeCase)
  {
    caseRow = index.row ();
  }
  else if (type == TypeTest)
  {
    testRow = index.row ();
    caseRow = index.parent ().row ();
    Q_ASSERT (caseRow >= 0);
  }
  else if (type == TypeDetail || type == TypeDetailError)
  {
    detailRow = index.row ();
    testRow = index.parent ().row ();
    Q_ASSERT (testRow >= 0);
    caseRow = index.parent ().parent ().row ();
    Q_ASSERT (caseRow >= 0);
  }
  return type;
}

TestModel::Type TestModel::getType(const QModelIndex &index) const
{
  if (!index.isValid ())
  {
    return TypeUnknown;
  }
  return Type (index.sibling (index.row (), ColumnType).data ().toInt ());
}

int TestModel::errorCount() const
{
  return errorCount_;
}

QList<QStandardItem *> TestModel::createRow(const QString &name, Type type) const
{
  QList<QStandardItem*> items;
  for (int i = 0; i < ColumnCount; ++i)
  {
    items << new QStandardItem;
  }
  items.first ()->setData (name, Qt::EditRole);
  items.last ()->setData (int (type), Qt::EditRole);
  return items;
}

void TestModel::setRowColor(const QModelIndex &index, const QColor &color)
{
  int row = index.row ();
  Q_ASSERT (index.isValid ());
  for (int i = 0; i < ColumnCount; ++i)
  {
    itemFromIndex (index.sibling (row, i))->setBackground (color);
  }
}

QString TestModel::title() const
{
  return title_;
}

void TestModel::setTitle(const QString &title)
{
  title_ = title;
}

void TestModel::clear()
{
  removeRows (0, rowCount ());
  errorCount_ = 0;
}

void TestModel::addNote(const QString &text)
{
  Q_ASSERT (!text.isEmpty ());
  QList<QStandardItem*> row = createRow (text, TypeNote);
  row.at(ColumnFailed)->setText (QLatin1String ("-1"));
  invisibleRootItem ()->appendRow (row);
  QModelIndex noteIndex = indexFromItem (row.first ());
  setRowColor (noteIndex, noteColor);
}

void TestModel::addCase(const QString &name)
{
  Q_ASSERT (!name.isEmpty ());
  invisibleRootItem ()->appendRow (createRow (name, TypeCase));
}

void TestModel::addTest(const QString &name, const QString &caseName)
{
  Q_ASSERT (!name.isEmpty ());
  Q_ASSERT (!caseName.isEmpty ());
  QModelIndex caseIndex = this->caseIndex (caseName);
  Q_ASSERT (caseIndex.isValid ());
  itemFromIndex (caseIndex)->appendRow (createRow (name, TypeTest));
}

void TestModel::addTestDetail(const QString &name, const QString &caseName,
                               const QString &detail)
{
  Q_ASSERT (!name.isEmpty ());
  Q_ASSERT (!caseName.isEmpty ());
  Q_ASSERT (!detail.isEmpty ());
  QModelIndex testIndex = this->testIndex (name, caseName);
  Q_ASSERT (testIndex.isValid ());
  itemFromIndex (testIndex)->appendRow (createRow (detail, TypeDetail));
}

void TestModel::addTestError(const QString &name, const QString &caseName, const QString &detail, const QString &file, int line)
{
  Q_ASSERT (!name.isEmpty ());
  Q_ASSERT (!caseName.isEmpty ());
  Q_ASSERT (!detail.isEmpty ());
  Q_ASSERT (!file.isEmpty ());
  QModelIndex testIndex = this->testIndex (name, caseName);
  Q_ASSERT (testIndex.isValid ());
  QList<QStandardItem*> row = createRow (detail, TypeDetailError);
  row.at (ColumnFile)->setText (file);
  row.at (ColumnLine)->setText (QString::number (line));
  itemFromIndex (testIndex)->appendRow (row);
  ++errorCount_;
}

void TestModel::updateTest(const QString &name, const QString &caseName,
                            bool isOk, int time)
{
  Q_ASSERT (!name.isEmpty ());
  Q_ASSERT (!caseName.isEmpty ());
  QModelIndex testIndex = this->testIndex (name, caseName);
  Q_ASSERT (testIndex.isValid ());
  int row = testIndex.row ();
  setData (testIndex.sibling (row, ColumnPassed), isOk ? 1 : 0);
  setData (testIndex.sibling (row, ColumnFailed), isOk ? 0 : 1);
  setData (testIndex.sibling (row, ColumnTime), time);
  setRowColor (testIndex, isOk ? goodColor : badColor);
  if (!isOk)
  {
    // Set ColumnFailed = -1 for fail messages to filter it later in proxy model.
    for (int i = 0, end = rowCount (testIndex); i < end; ++i)
    {
      QModelIndex child = testIndex.child (i, ColumnFailed);
      if (child.data ().isNull ())
      {
        setData (child, -1);
      }
    }
  }
}

void TestModel::updateCase(const QString &name, int passedCount,
                            int failedCount, int time)
{
  Q_ASSERT (!name.isEmpty ());
  QModelIndex caseIndex = this->caseIndex (name);
  Q_ASSERT (caseIndex.isValid ());
  int row = caseIndex.row ();
  setData (caseIndex.sibling (row, ColumnPassed), passedCount);
  setData (caseIndex.sibling (row, ColumnFailed), failedCount);
  setData (caseIndex.sibling (row, ColumnTime), time);
  setRowColor (caseIndex, (failedCount == 0) ? goodColor : badColor);
}

void TestModel::renameTest(const QString &oldName, const QString &newName,
                           const QString &caseName)
{
  Q_ASSERT (!oldName.isEmpty ());
  Q_ASSERT (!caseName.isEmpty ());
  QModelIndex testIndex = this->testIndex (oldName, caseName);
  Q_ASSERT (testIndex.isValid ());
  Q_ASSERT (!newName.isEmpty ());
  setData (testIndex, newName);
}
