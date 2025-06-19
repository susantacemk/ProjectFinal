#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include<QTreeView>
#include<QPushButton>
#include<QLabel>
#include<QHBoxLayout>
#include<QTableWidget>
#include<QStandardItemModel>
#include<fileexplorer.h>
#include<codeeditor.h>
#include<terminal_widget.h>
#include <QCompleter>
#include<QSet>
#include <QStringListModel>
#include<utils/compilationmanager.h>
#include<utils/lspclient.h>
#include<utils/runterminalwidget.h>
#include<utils/powershellterminalwidget.h>
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
    QString filePaths;


private slots:
    void on_editor_tab_tabCloseRequested(int index);

    void on_terminal_tab_tabCloseRequested(int index);

    void on_actionSymbol_Table_triggered();

    void on_actionFile_Explorer_triggered();

    void on_actionTerminal_triggered();

    void customizeUi(); // set up ui combobox
    void setUpEditorPanel(); // set up editor panel
    bool isTreeViewEmpty(QTreeView *treeView); // check the treeview are empty or not
    void setUpTreeView(QString Dir); // checking last opened folder

    void on_minimize_structure_btn_clicked();

    void on_hide_btn_clicked();

    void on_customize_combo_currentIndexChanged(int index);

    void fileDoubleClicked(const QModelIndex &index);

    void updateStatusBar(const QString &filePath); // update status bar

    void on_actionNew_Text_File_triggered();

    void on_actionSAve_triggered();

    void on_actionSave_As_triggered();

    void on_actionNew_File_triggered();

    void on_actionNew_Package_triggered();

    void on_actionOpen_File_triggered();

    void on_actionOpen_Folder_triggered();

    void updateRecentFoldersComboBox(const QString &newPath);

    void on_recently_opened_combo_activated(int index);

    void on_actionShow_History_triggered();

    void on_actionSave_All_triggered();

    void on_actionClose_Projects_triggered();

    void on_actionExit_triggered();

    void updateFileContent(const QString &newContent);

    void onTextChanged();

    void markTabUnsaved(int index);

    void markTabSaved(int index);

    void closeAllTabsWithPrompt();

    void closeSavedTabs();

    void toggleLockTab();

    CodeEditor* currentEditor() const;



    void on_terminal_hide_btn_clicked();

    void on_more_btn_clicked();

    void on_actionUndo_triggered();

    void on_actionRedo_triggered();

    void on_actionCut_triggered();

    void on_actionCopy_triggered();

    void on_actionPaste_triggered();

    void on_actionSelect_All_triggered();

    void fileDoubleClickedFromFilePath(const QString &filePath,int lineNumber);

    void on_actionSearch_triggered();


    void handleDiagnostics(const QJsonArray &diagnostics);

    void on_actionProblems_triggered();

    void on_problems_btn_clicked();

    void setupProblemsList();

    void on_terminal_btn_clicked();

    void populateSymbolTable(const QJsonArray &symbols);

    void setupSymbolTable();

    void parseAndAddSymbols(const QJsonArray &symbols, const QString &parentScope, QSet<QString> &uniqueKeys);

    void on_actionNew_Projects_triggered();


    void on_actionOpen_Projects_triggered();

    void createProjectMarker(const QString &projectPath, const QString &projectName, const QString &language);

    void on_actionRecent_Projects_triggered();

    void saveToRecentProjects(const QString &projectPath);

    void on_actionClose_Projects_2_triggered();

    void on_actionRename_Symbols_triggered();

    void on_actionChange_All_Occurances_triggered();
    void applyRenameEdits(const QJsonObject &changes);
    CodeEditor* getEditorByUri(const QString &uri);

    void on_actionRename_Filename_triggered();

    void on_actionClose_Tab_triggered();

    void on_actionClose_Other_Tabs_triggered();

    void on_actionClose_All_Tabs_triggered();

    void on_actionClose_Tabs_To_The_Left_triggered();

    void on_actionClose_Tabs_To_The_Right_triggered();

    void on_actionLock_Tab_triggered();

    void on_actionZoom_In_triggered();

    void on_actionZoom_Out_triggered();

    void on_actionReset_Zoom_triggered();

    void on_actionFull_Screen_triggered();



    void on_actionZen_Mode_triggered();

    void on_actionToggle_Menubar_triggered();

    void on_actionPrimary_SideBar_triggered();

    void on_actionSecond_Sidebar_triggered();

    void on_actionStatus_Bar_triggered();

    void on_actionWord_Wrap_toggled(bool arg1);

    void on_actionCompilation_triggered();

    void on_actionOpen_Terminal_triggered();

    void on_actionRun_Without_Debugging_triggered();

    void on_run_btn_clicked();

    // optional
    void onEditorTabChanged(int index);

protected:
    void keyPressEvent(QKeyEvent *event) override;
private:
    Ui::MainWindow *ui;
    FileExplorer *explorer;
    QString lastOpenedDirectory;
    QStringList recentFolders;
    QLabel *fileStatusLabel;
    QLabel *iconLabel;
    QFileSystemModel *model;


    // file menu setup
    QStringList supportedExtensions = { "c", "cpp", "h", "py", "txt" };
    QVector<QString> openFileExtensions; // Add this beside openFilePaths
    QVector<QString> openFilePaths;

    // QString filePaths;

    // checked saved and unsaved file
    QMap<int, bool> tabUnsavedMap;  // tab index → true if unsaved

    // clangD

    CodeEditor *editor;


    // Terminal

    //TerminalWidget *terminal;



    // more btn
    bool isExpanded = false;
    QHBoxLayout *extraButtonLayout = nullptr; // Layout for additional buttons
    QPushButton *closeAll;
    QPushButton *closeSaved;
    QPushButton *lockTab;
    QWidget *popupWidget;
    QMap<int, bool> tabLockedMap;



    QCompleter *completer;
    QStringListModel *completionModel;



    LSPClient *lspClient=nullptr;
    QTimer *diagnosticsTimer = nullptr;


    // problem list
    QTableWidget *problemsTable;
    QMap<int, QString> problemLines; // Maps line → message (for removal)

    // symbol table

    QTableView *symbolTableView;
    QStandardItemModel *symbolTableModel;


    // project
    QString currentProjectPath;
    QFileSystemModel *projectModel;

    // rename symbol
    QMap<QString, CodeEditor*> openEditors;


    // zen mode
    bool isZenMode = false;
    QAction *zenModeAction;
    QToolBar *toolBar;      // your activity/tool bar
    QWidget *sideBar;       // if you have file explorer etc.
    QTabWidget *tabBar;        // or QTabWidget* tabWidget;



    // compiler works
    CompilationManager *compilationManager = nullptr;
    TerminalWidget *compilationTerminal = nullptr;


    void setupCompilationTerminal();  // create tab
    void compileCurrentFile();        // trigger


    // run options

    RunTerminalWidget *runTerminalWidget;


    // powershell
    PowerShellTerminalWidget *terminal1;

};
#endif // MAINWINDOW_H
