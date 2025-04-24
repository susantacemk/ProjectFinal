#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_editor_tab_tabCloseRequested(int index);

    void on_terminal_tab_tabCloseRequested(int index);

    void on_actionSymbol_Table_triggered();

    void on_actionFile_Explorer_triggered();

    void on_actionTerminal_triggered();

    void customizeUi(); // set up ui combobox
    void setUpEditorPanel(); // set up editor panel

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
