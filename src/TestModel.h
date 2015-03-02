#ifndef TESTMODEL_H
#define TESTMODEL_H

#include <QStandardItemModel>

namespace QtcGtest {
  namespace Internal {

    /*!
     * \brief Model representing results of tests run.
     */
    class TestModel : public QStandardItemModel
    {
        Q_OBJECT
      public:
        enum Column
        {
          ColumnName = 0, ColumnPassed, ColumnFailed, ColumnTime, ColumnType,
          ColumnCount,
          ColumnFile = ColumnPassed, // For error.
          ColumnLine = ColumnFailed // For error.
        };
        enum Type
        {
          TypeCase, TypeTest, TypeDetail, TypeDetailError, TypeNote, TypeUnknown
        };

      public:
        explicit TestModel(QObject *parent = 0);

        QModelIndex findItem (const QString& name, const QModelIndex& parent) const;

        QModelIndex caseIndex (const QString& name) const;
        QModelIndex testIndex (const QString& name, const QString& caseName) const;

        QModelIndex previousError (const QModelIndex& index) const;
        QModelIndex nextError (const QModelIndex& index) const;

        Type getType (const QModelIndex& index) const;

        int errorCount () const;

        QString title() const;
        void setTitle(const QString &title);

        void clear ();

      public slots:
        void addNote (const QString& text);
        void addCase (const QString& name);
        void addTest (const QString& name, const QString& caseName);
        void addTestDetail (const QString& name, const QString& caseName, const QString& detail);
        void addTestError (const QString& name, const QString& caseName, const QString& detail, const QString& file, int line);

        void updateTest (const QString& name, const QString& caseName, bool isOk, int time);
        void updateCase (const QString& name, int passedCount, int failedCount, int time);

        void renameTest (const QString& oldName, const QString& newName, const QString& caseName);

      private:
        QList<QStandardItem*> createRow (const QString& name, Type type) const;
        void setRowColor (const QModelIndex& index, const QColor& color);
        Type getCurrentRows (const QModelIndex& index, int& caseRow, int& testRow, int& detailRow) const;

      private:
        QString title_;
        int errorCount_;
        QHash<int, QString> columnNames_;
    };

  } // namespace Internal
} // namespace QtcGtest

#endif // TESTMODEL_H
