#ifndef TESTPROJECT_H
#define TESTPROJECT_H

#include <QObject>

namespace ProjectExplorer
{
  class Project;
  class RunConfiguration;
}

namespace QtcGtest {
  namespace Internal {

    class CustomRunConfiguration;

    class TestProject : public QObject
    {
        Q_OBJECT
      public:
        explicit TestProject(QObject *parent = 0);

      public slots:

        void checkProject ();
        void checkChanged ();
        void checkCurrent ();

        void handleDocumentsChange (const QModelIndex &topLeft, const QModelIndex &bottomRight,
                                    const QVector<int> &roles);
        void handleDocumentsClose (const QModelIndex &parent, int start, int end);

      private:
        QStringList getChangedFiles (int beginRow, int endRow, bool modifiedFlag) const;
        void runTestsForFiles (const QStringList& files, CustomRunConfiguration* configuration) const;
        CustomRunConfiguration *parse(ProjectExplorer::Project *project);
        QString gtestMainInclude () const;
        void preprocessDependencyTable ();
        void runTests (CustomRunConfiguration *configuration) const;
        QStringList getTestCases (const QSet<QString> &fileNames) const;
        QStringList getDependentFiles (const QStringList& files) const;

      private:
        QStringList changedFiles_;
        QString gtestIncludeFile_;
        QHash<QString, QStringList> dependencyTable_;
    };

  } // namespace Internal
} // namespace QtcGtest

#endif // TESTPROJECT_H
