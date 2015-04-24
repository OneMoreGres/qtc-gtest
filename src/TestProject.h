#ifndef TESTPROJECT_H
#define TESTPROJECT_H

#include <QObject>
#include <QHash>
#include <QStringList>

#include <utils/fileutils.h>

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
        Utils::FileNameList getChangedFiles(int beginRow, int endRow, bool modifiedFlag) const;
        void runTestsForFiles (const Utils::FileNameList &files, CustomRunConfiguration* configuration) const;
        CustomRunConfiguration *parse(ProjectExplorer::Project *project);
        Utils::FileNameList gtestMainIncludes() const;
        void preprocessDependencyTable ();
        void runTests (CustomRunConfiguration *configuration) const;
        QStringList getTestCases (const QSet<Utils::FileName> &fileNames) const;
        Utils::FileNameList getDependentFiles(const Utils::FileNameList &files) const;

      private:
        Utils::FileNameList changedFiles_;
        Utils::FileNameList gtestIncludeFiles_;
        QHash<Utils::FileName, Utils::FileNameList> dependencyTable_;
        QHash<QString, QString> testFilterPatterns_;
    };

  } // namespace Internal
} // namespace QtcGtest

#endif // TESTPROJECT_H
