#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QLabel>
#include <QDateTime>
#include <QTimer>
#include<QStandardItemModel>
#include<QStandardItem>
#include <codeeditor.h>
#include<chighlighter.h>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // Assuming `this` is a QMainWindow

    ui->symbol_table->setVisible(false);
    // intially all tab are clear
    ui->editor_tab->clear();
    ui->terminal_tab->clear();

    // method call
    customizeUi();
    setUpEditorPanel();
}

MainWindow::~MainWindow()
{
    delete ui;
}

// tab close requested
void MainWindow::on_editor_tab_tabCloseRequested(int index)
{
    QWidget *tab = ui->editor_tab->widget(index);
    ui->editor_tab->removeTab(index);
    delete tab;
}

// terminal tab closed
void MainWindow::on_terminal_tab_tabCloseRequested(int index)
{
    ui->terminal_tab->removeTab(index);
}

// symbol table close and open

void MainWindow::on_actionSymbol_Table_triggered()
{
    bool isVisible = ui->symbol_table->isVisible();
    ui->symbol_table->setVisible(!isVisible);
}


// left sidebar hide or show
void MainWindow::on_actionFile_Explorer_triggered()
{
    bool isVisible = ui->left_widget->isVisible();
    ui->left_widget->setVisible(!isVisible);
}

// terminal panel hide and show
void MainWindow::on_actionTerminal_triggered()
{
    bool isVisible = ui->terminal_widget->isVisible();
    ui->terminal_widget->setVisible(!isVisible);
}

// customize all combobox
void MainWindow::customizeUi(){
    // modify the tree structure
    QStandardItemModel *model = qobject_cast<QStandardItemModel *>(ui->customize_combo->model());
    QStandardItem *firstItem = model->item(0);
    firstItem->setFlags(firstItem->flags() & ~Qt::ItemIsEnabled);
    firstItem->setForeground(QBrush(Qt::gray));
    firstItem->setFont(QFont("Segoe UI", 9, QFont::Normal, true)); // Italic

    // modify the list of recently opened project

    QStandardItemModel *model1=qobject_cast<QStandardItemModel *>(ui->recently_opened_combo->model());
    QStandardItem *model1FirstItem = model1->item(0);
    model1FirstItem->setFlags(model1FirstItem->flags() & ~Qt::ItemIsEnabled);
    model1FirstItem->setForeground(QBrush(Qt::gray));
    model1FirstItem->setFont(QFont("Segoe UI", 9, QFont::Normal, true)); // Italic


    // modify the recent configured list

    QStandardItemModel *model2=qobject_cast<QStandardItemModel *>(ui->recentl_configured_combo->model());
    QStandardItem *model2FirstItem = model2->item(0);
    model2FirstItem->setFlags(model2FirstItem->flags() & ~Qt::ItemIsEnabled);
    model2FirstItem->setForeground(QBrush(Qt::gray));
    model2FirstItem->setFont(QFont("Segoe UI", 9, QFont::Normal, true)); // Italic


}


// setup method of code editor section

void  MainWindow:: setUpEditorPanel(){
    // initially all tab are removed
    CodeEditor *editor = new CodeEditor(this);
    int tabIndex = ui->editor_tab->addTab(editor, "Untitled");
    ui->editor_tab->setCurrentIndex(tabIndex);
    CHighlighter *highlighter = new CHighlighter(editor->document());
}

