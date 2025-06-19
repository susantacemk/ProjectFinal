// fileexplorer.cpp
#include "fileexplorer.h"
#include <QFileInfo>
#include<QTreeView>
#include<QMimeDatabase>
#include<QMessageBox>
#include <codeeditor.h>
#include<mainwindow.h>
FileExplorer::FileExplorer(QTreeView *externalView, QObject *parent)
    : QObject(parent), model(new QFileSystemModel(this)), treeView(externalView)
{
    model->setFilter(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::Files);
    model->setNameFilterDisables(false);
    treeView->setModel(model);
    treeView->hideColumn(1);
    treeView->hideColumn(2);
    treeView->hideColumn(3);

    // call the signal


}

void FileExplorer::setRootPath(const QString &path)
{
    QModelIndex rootIndex = model->setRootPath(path);
    treeView->setRootIndex(rootIndex);
}
