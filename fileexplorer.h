// fileexplorer.h
#ifndef FILEEXPLORER_H
#define FILEEXPLORER_H

#include <QObject>
#include <QFileSystemModel>
#include <QTreeView>
#include<QModelIndex>
class FileExplorer : public QObject
{
    Q_OBJECT

public:
    explicit FileExplorer(QTreeView *externalView, QObject *parent = nullptr);
    void setRootPath(const QString &path);

signals:

private:
    QFileSystemModel *model;
    QTreeView *treeView;
};

#endif // FILEEXPLORER_H
