#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QLabel>
#include <QDateTime>
#include <QTimer>
#include<QStandardItemModel>
#include<QStandardItem>
#include<QFileDialog>
#include <codeeditor.h>
#include<chighlighter.h>
#include<cpphighlighter.h>
#include<pythonhighlighter.h>
#include<fileexplorer.h>




#include<QJsonObject>
#include<QJsonDocument>
#include<QSettings>
#include<QFileSystemModel>
#include<QTextCursor>
#include<QMimeDatabase>
#include<QMessageBox>
#include<QVariant>
#include<QWidget>
#include<QPixmap>
#include<QFrame>
#include<terminal_widget.h>
#include<QInputDialog>
#include<QDialog>
#include<QListWidget>
#include<QDockWidget>
#include<QToolTip>
#include<QJsonArray>
#include<QJsonObject>



#include<utils/powershellterminalwidget.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow),editor(new CodeEditor(nullptr))
   // ,terminal(new TerminalWidget(this, TerminalWidget::DefaultMode))
    // initialized clangD
{
    ui->setupUi(this);
    // Assuming `this` is a QMainWindow
    editor->installEventFilter(this);
    // setup status bar
    fileStatusLabel = new QLabel(this);
    iconLabel = new QLabel(this);
    fileStatusLabel->setStyleSheet("color: white;border:none;"); // optional: make it pretty
    iconLabel->setFixedSize(24, 24);  // Small icon size
    iconLabel->setAlignment(Qt::AlignCenter);
    ui->statusBar->addWidget(fileStatusLabel); // left side file path shown
    QFrame *separator = new QFrame(this);
    separator->setFrameShape(QFrame::VLine);
    separator->setFrameShadow(QFrame::Sunken);
    ui->statusBar->addPermanentWidget(separator);
    ui->statusBar->addPermanentWidget(iconLabel); // right side


    ui->symbol_table->setVisible(false); // initially symbol table is not visible
    ui->terminal_widget->setVisible(false);    // initially terminal are off


    // intially all tab are clear
    ui->editor_tab->clear();
    ui->terminal_tab->clear();
    // initially terminal are off


    // last opened folder
    QSettings settings("abc", "def");
    QString lastDir= settings.value("lastOpenedDirectory").toString();
    if(!lastDir.isEmpty()){
        // call one method
        setUpTreeView(lastDir);
    }

    // method call
    customizeUi();
    //setUpEditorPanel();


    // connect the signal
    // file double clicked
    connect(ui->fileTreeView, &QTreeView::doubleClicked, this, &MainWindow::fileDoubleClicked);
    //connect(editor,&QPlainTextEdit::textChanged,this,&MainWindow::onTextChanged);

    // recently Opened combo box setup
    recentFolders = settings.value("recentFolders").toStringList();
    ui->recently_opened_combo->addItems(recentFolders);


    // projects
    projectModel = new QFileSystemModel(this);
    projectModel->setReadOnly(true);
    projectModel->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Files);







    // More Button created and set the layout
    popupWidget = nullptr;
    // Create the popup widget dynamically
    popupWidget = new QWidget(this, Qt::Popup);
    QVBoxLayout *layout = new QVBoxLayout(popupWidget);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(10);

    popupWidget->setCursor(Qt::PointingHandCursor);
    // stylesheet of these three more button
    // Common style
    QString btnStyle = R"(
    QPushButton {
        background-color: #f0f0f0;
        border: 1px solid #ccc;
        border-radius: 3px;
        padding: 4px 8px;
        font-size: 14px;
        color: #333;
    }
    QPushButton:hover {
        background-color: #e0e0e0;
        border-color: #999;
    }
    QPushButton:pressed {
        background-color: #d0d0d0;
    })";
    closeAll = new QPushButton("Close All",popupWidget);
    closeSaved = new QPushButton("Close Saved",popupWidget);
    lockTab = new QPushButton("Lock Tab",popupWidget);
    closeAll->setStyleSheet(btnStyle);
    closeSaved->setStyleSheet(btnStyle);
    lockTab->setStyleSheet(btnStyle);
    layout->addWidget(closeAll);
    layout->addWidget(closeSaved);
    layout->addWidget(lockTab);

    popupWidget->setLayout(layout);
    popupWidget->adjustSize();

    // connect

    connect(ui->editor_tab, &QTabWidget::currentChanged, this, &MainWindow::onEditorTabChanged);


    // connect signal for more button
    connect(closeAll,&QPushButton::clicked,this,[=]{
        closeAllTabsWithPrompt();
        popupWidget->hide();
        ui->more_btn->setChecked(false);
    });
    connect(closeSaved, &QPushButton::clicked, this, &MainWindow::closeSavedTabs);
    connect(lockTab, &QPushButton::clicked, this, &MainWindow::toggleLockTab);


    // client - server
    lspClient = new LSPClient(this);

    connect(lspClient, &LSPClient::diagnosticsReceived, this, &MainWindow::handleDiagnostics);

    connect(lspClient, &LSPClient::completionsReceived, this, [=](const QStringList &suggestions) {
        if (CodeEditor *activeEditor = currentEditor()) {
            activeEditor->showCompletionPopup(suggestions);
        }
    });

    diagnosticsTimer = new QTimer(this);
    diagnosticsTimer->setInterval(300);  // debounce delay
    diagnosticsTimer->setSingleShot(true);

    connect(diagnosticsTimer, &QTimer::timeout, this, [=]() {
        if (editor && lspClient) {
            QString uri = QUrl::fromLocalFile(filePaths).toString();
            QString content = editor->toPlainText();
            lspClient->sendDidChange(uri, content);
            // qDebug() << " Debounced didChange sent to clangd";
        }
    });

    lspClient->start(lastDir);  // Start clangd after everything is connected
    connect(lspClient, &LSPClient::renameEditsReady, this, &MainWindow::applyRenameEdits);



    setupProblemsList();
    setupSymbolTable();
    connect(lspClient, &LSPClient::documentSymbolsReceived, this, &MainWindow::populateSymbolTable);

    compilationTerminal = new TerminalWidget();
    runTerminalWidget = new RunTerminalWidget();



    // powershell implementation
    terminal1 = new PowerShellTerminalWidget();
}

void MainWindow::setupSymbolTable(){
    ui->symbol_table->verticalHeader()->setVisible(false);
    symbolTableView = ui->symbol_table;  // if using from .ui
    symbolTableModel = new QStandardItemModel(this);

    // Set 5 headers
    symbolTableModel->setColumnCount(5);
    symbolTableModel->setHorizontalHeaderLabels(QStringList()
                                                << "Line" << "Name" << "Type" << "Scope" << "Category");

    symbolTableView->setModel(symbolTableModel);
    symbolTableView->horizontalHeader()->setStretchLastSection(true);
    symbolTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    symbolTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    connect(symbolTableView, &QTableView::clicked, this, [=](const QModelIndex &index) {
        int row = index.row();
        QModelIndex lineIndex = symbolTableModel->index(row, 0);
        int line = symbolTableModel->data(lineIndex, Qt::UserRole).toInt();

        if (CodeEditor *editor = currentEditor()) {
            QTextCursor cursor = editor->textCursor();
            cursor.movePosition(QTextCursor::Start);
            cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, line);
            editor->setTextCursor(cursor);
            editor->centerCursor();
        }
    });


}
void MainWindow::setupProblemsList(){
    // // problems list
    problemsTable = new QTableWidget();
    problemsTable->setColumnCount(3);
    problemsTable->setHorizontalHeaderLabels(QStringList() << "Line" << "Type" << "Message");
    problemsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    problemsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    problemsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    problemsTable->setFont(QFont("Consolas", 10));
    problemsTable->setShowGrid(false);
    problemsTable->verticalHeader()->setVisible(false);
    problemsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    problemsTable->setAlternatingRowColors(true); // optional
    problemsTable->setStyleSheet(R"(
    QTableWidget {
        background-color: #1e1e1e;
        color: #d4d4d4;
        border: none;
        gridline-color: #444;
    }
    QHeaderView::section {
        background-color: #333;
        color: white;
        padding: 4px;
        font-weight: bold;
    }
    QTableWidget::item:selected {
        background-color: #264f78;
        color: white;
    }
)");

    connect(problemsTable, &QTableWidget::cellClicked, this, [=](int row, int column) {
        QString lineStr = problemsTable->item(row, 0)->text();
        int lineNumber = lineStr.toInt();

        if (CodeEditor *e = currentEditor()) {
            QTextCursor cursor = e->textCursor();
            cursor.movePosition(QTextCursor::Start);
            cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, lineNumber - 1);
            e->setTextCursor(cursor);
            e->centerCursor();
        }
    });
}

void MainWindow::handleDiagnostics(const QJsonArray &diagnostics)
{
    if (!editor) return;
    problemsTable->setRowCount(0);  // Clear old errors
    editor->clearDiagnostics();

    for (const QJsonValue &diagVal : diagnostics) {
        QJsonObject diag = diagVal.toObject();
        QJsonObject range = diag["range"].toObject();
        QJsonObject start = range["start"].toObject();
        QJsonObject end = range["end"].toObject();

        int startLine = start["line"].toInt();
        int startChar = start["character"].toInt();
        int endLine = end["line"].toInt();
        int endChar = end["character"].toInt();

        //  Expand zero-length range to at least 1 char
        if (startLine == endLine && startChar == endChar) {
            endChar = startChar + 1;
        }

        QString message = diag["message"].toString();
        QString severity = diag["severity"].toInt() == 1 ? "Error" : "Warning";
        QString fullMessage = "[" + severity + "] " + message;
        // Get the line text to check for comment
        QTextBlock block = editor->document()->findBlockByNumber(startLine+1);
        QString lineText = block.text().trimmed();
        //  Skip if line is a comment (// or /* */)
        if (lineText.startsWith("//") || lineText.startsWith("/*") || lineText.contains("*/")) {
            editor->addDiagnostic(startLine+1, startChar, endLine, endChar, severity,fullMessage);
            // continue;
        }

        editor->addDiagnostic(startLine, startChar, endLine, endChar,severity, fullMessage);

        // Add to table
        int row = problemsTable->rowCount();
        problemsTable->insertRow(row);

        // problemsTable->setItem(row, 0, new QTableWidgetItem(QString::number(startLine + 1)));
        // problemsTable->setItem(row, 1, new QTableWidgetItem(severity));
        // problemsTable->setItem(row, 2, new QTableWidgetItem(message));

        QTableWidgetItem *lineItem = new QTableWidgetItem(QString::number(startLine + 1));
        lineItem->setTextAlignment(Qt::AlignCenter);
        lineItem->setForeground(Qt::darkGray);

        QTableWidgetItem *typeItem = new QTableWidgetItem(severity);
        QTableWidgetItem *msgItem  = new QTableWidgetItem(message);

        // Set severity-based coloring
        if (severity == "Error") {
            typeItem->setForeground(QBrush(Qt::red));
            msgItem->setForeground(QBrush(Qt::red));
        } else {
            typeItem->setForeground(QBrush(QColor(230,184, 0)));  // yellow-ish
            msgItem->setForeground(QBrush(QColor(230,184,0)));
        }

        problemsTable->setItem(row, 0, lineItem);
        problemsTable->setItem(row, 1, typeItem);
        problemsTable->setItem(row, 2, msgItem);

    }

}

MainWindow::~MainWindow()
{
    delete ui;
    delete lspClient;
}


void MainWindow::fileDoubleClickedFromFilePath(const QString &filePath,int lineNumber){
    QFileInfo fileInfo(filePath);

    if (!fileInfo.isFile())
        return;

    // Check MIME type (text only)
    QMimeDatabase mimeDb;
    QMimeType mimeType = mimeDb.mimeTypeForFile(fileInfo);
    if (!mimeType.name().startsWith("text/")) {
        QMessageBox::warning(this, "Not a Text File", "Only text files can be opened.");
        return;
    }

    // 🌟 Check all tabs if file is already open
    for (int i = 0; i < ui->editor_tab->count(); ++i) {
        QWidget *widget = ui->editor_tab->widget(i);
        if (widget) {
            QVariant openedFilePath = widget->property("filePath");
            if (openedFilePath.isValid() && openedFilePath.toString() == filePath) {
                // Found already opened file
                ui->editor_tab->setCurrentIndex(i);
                return;
            }
        }
    }

    // 🌟 File not open yet - open it now
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        QString content = in.readAll();
        file.close();

        editor = new CodeEditor(this);
        // 🌟 Prevent premature textChanged signal
        editor->blockSignals(true);
        editor->setPlainText(content);
        editor->blockSignals(false);


        // Save file path as dynamic property
        editor->setProperty("filePath", filePath);
        editor->setCurrentFilePath(filePath);

        connect(editor, &CodeEditor::requestCompletion, this, [=](const QString &uri, int line, int character) {
            if (lspClient) {
                lspClient->sendCompletionRequest(uri, line, character);
            }
        });

        connect(editor,&CodeEditor::textChanged,this,&MainWindow::onTextChanged);
        // 🌟 Determine icon based on file extension
        QString suffix = fileInfo.suffix().toLower();
        QIcon fileIcon;

        if (suffix == "cpp" || suffix == "h" || suffix == "hpp") {
            fileIcon = QIcon(":/c++.png"); // C++ icon
            new CppHighlighter(editor->document());
        } else if (suffix == "py") {
            fileIcon = QIcon(":/python.png"); // Python icon
            new PythonHighlighter(editor->document());
        } else if (suffix == "txt") {
            fileIcon = QIcon(":/txt.png"); // Text file icon
        } else if (suffix == "c") {
            fileIcon = QIcon(":/c.png"); // c file icon
            new CHighlighter(editor->document());
        }
        QString uri = QUrl::fromLocalFile(filePath).toString();
        lspClient->sendDidOpen(uri, suffix, content);
        // symbol table
        lspClient->requestDocumentSymbols(uri);

        int tabIndex = ui->editor_tab->addTab(editor,fileIcon, fileInfo.fileName());
        ui->editor_tab->setCurrentIndex(tabIndex);
        tabUnsavedMap[tabIndex] = false; // initially saved
        QTimer::singleShot(0, this, [=]() {
            connect(editor, &QPlainTextEdit::textChanged, this, [=]() {
                markTabUnsaved(tabIndex);
            });
        });
        openFilePaths.insert(tabIndex,filePath);
        openFileExtensions.insert(tabIndex,suffix);
        updateStatusBar(filePath);
        filePaths=filePath;
        QWidget *editorWidget = ui->editor_tab->currentWidget();
        if (auto *editor = qobject_cast<QPlainTextEdit *>(editorWidget)) {
            QTextCursor cursor = editor->textCursor();
            cursor.movePosition(QTextCursor::Start);
            for (int i = 1; i < lineNumber; ++i)
                cursor.movePosition(QTextCursor::Down);
            editor->setTextCursor(cursor);
            editor->centerCursor();
        }
        openEditors[filePath]=editor;
}
}
void MainWindow::fileDoubleClicked(const QModelIndex &index)
{
    model = qobject_cast<QFileSystemModel*>(ui->fileTreeView->model());
    if (!model) return;

    QString filePath = model->filePath(index);
    QFileInfo fileInfo(filePath);

    if (!fileInfo.isFile())
        return;

    // Check MIME type (text only)
    QMimeDatabase mimeDb;
    QMimeType mimeType = mimeDb.mimeTypeForFile(fileInfo);
    if (!mimeType.name().startsWith("text/")) {
        QMessageBox::warning(this, "Not a Text File", "Only text files can be opened.");
        return;
    }

    // 🌟 Check all tabs if file is already open
    for (int i = 0; i < ui->editor_tab->count(); ++i) {
        QWidget *widget = ui->editor_tab->widget(i);
        if (widget) {
            QVariant openedFilePath = widget->property("filePath");
            if (openedFilePath.isValid() && openedFilePath.toString() == filePath) {
                // Found already opened file
                ui->editor_tab->setCurrentIndex(i);
                return;
            }
        }
    }

    // 🌟 File not open yet - open it now
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        QString content = in.readAll();
        file.close();

        editor = new CodeEditor(this);
        // 🌟 Prevent premature textChanged signal
        editor->blockSignals(true);
        editor->setPlainText(content);
        editor->blockSignals(false);


        // Save file path as dynamic property
        editor->setProperty("filePath", filePath);
        editor->setCurrentFilePath(filePath);

        connect(editor, &CodeEditor::requestCompletion, this, [=](const QString &uri, int line, int character) {
            if (lspClient) {
                lspClient->sendCompletionRequest(uri, line, character);
            }
        });

        connect(editor,&CodeEditor::textChanged,this,&MainWindow::onTextChanged);
        // 🌟 Determine icon based on file extension
        QString suffix = fileInfo.suffix().toLower();
        QIcon fileIcon;

        if (suffix == "cpp" || suffix == "h" || suffix == "hpp") {
            fileIcon = QIcon(":/c++.png"); // C++ icon
            new CppHighlighter(editor->document());
        } else if (suffix == "py") {
            fileIcon = QIcon(":/python.png"); // Python icon
             new PythonHighlighter(editor->document());
        } else if (suffix == "txt") {
            fileIcon = QIcon(":/txt.png"); // Text file icon
        } else if (suffix == "c") {
            fileIcon = QIcon(":/c.png"); // c file icon
            new CHighlighter(editor->document());
        }
        QString uri = QUrl::fromLocalFile(filePath).toString();
        lspClient->sendDidOpen(uri, suffix, content);
        // symbol table
        lspClient->requestDocumentSymbols(uri);


        int tabIndex = ui->editor_tab->addTab(editor,fileIcon, fileInfo.fileName());
        ui->editor_tab->setCurrentIndex(tabIndex);
        tabUnsavedMap[tabIndex] = false; // initially saved
        QTimer::singleShot(0, this, [=]() {
            connect(editor, &QPlainTextEdit::textChanged, this, [=]() {
                markTabUnsaved(tabIndex);
            });
        });
        openFilePaths.insert(tabIndex,filePath);
        openFileExtensions.insert(tabIndex,suffix);
        updateStatusBar(filePath);
        filePaths=filePath;
    }
    openEditors[filePath]=editor;
}




// update status bar
void MainWindow::updateStatusBar(const QString &filePath) {
    QFileInfo fileInfo(filePath);
    QString extension = fileInfo.suffix().toLower();

    QString languageIcon;
    if (extension == "cpp" || extension == "h" || extension == "hpp") {
        languageIcon = ":/c++.png"; // (your C++ icon path)
    } else if (extension == "py") {
        languageIcon = ":/python.png"; // (your Python icon path)
    } else if (extension == "txt") {
        languageIcon = ":/txt.png"; // (your text file icon path)
    } else {
        languageIcon = ":/c.png"; // c icon
    }

    QPixmap pixmap(languageIcon);
    pixmap = pixmap.scaled(16, 16, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    iconLabel->setPixmap(pixmap);
    fileStatusLabel->setText(" " + filePath);
}

// setup last opened tree view structure

void MainWindow::setUpTreeView(QString dir){
    explorer = new FileExplorer(ui->fileTreeView, this);
    ui->fileTreeView->header()->hide();
    explorer->setRootPath(dir);
    return;
}

// tab close requested
void MainWindow::on_editor_tab_tabCloseRequested(int index)
{
    // QWidget *tab = ui->editor_tab->widget(index);
    // ui->editor_tab->removeTab(index);
    // tab->deleteLater();

    QWidget *closingTab = ui->editor_tab->widget(index);
    if (closingTab) {
        on_actionSAve_triggered();
        closingTab->deleteLater();
        ui->fileTreeView->clearSelection();
        ui->editor_tab->removeTab(index);
    }

    // Re-trigger tab change manually
    int newIndex = ui->editor_tab->currentIndex();
    if (newIndex >= 0)
        onEditorTabChanged(newIndex);
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
    int tabCount = ui->editor_tab->count();
    if(isVisible==false){
        if(tabCount<=0){
            ui->symbol_table->setVisible(false);
            QMessageBox::information(this,"Information","No file is opened");
            ui->actionSymbol_Table->setChecked(false);
        }else
            ui->symbol_table->setVisible(true);
    }else{
        ui->symbol_table->setVisible(false);
    }
    //setupSymbolTable();
}



void MainWindow::on_actionFile_Explorer_triggered()
{
    bool isVisible = ui->left_widget->isVisible();
    bool isEmpty = isTreeViewEmpty(ui->fileTreeView);

    if (isEmpty && isVisible) {
       explorer = new FileExplorer(ui->fileTreeView, this);
       ui->fileTreeView->header()->hide();

        QSettings settings("abc", "def");
        QString lastDir = settings.value("lastOpenedDirectory").toString();
        QString dir;

        // Check if last directory exists
        if (!lastDir.isEmpty() && QFileInfo(lastDir).isDir()) {
            dir = lastDir;
            lastOpenedDirectory=dir;
        } else {
            dir = QFileDialog::getExistingDirectory(this, "Open Folder", QDir::homePath());
            lastOpenedDirectory=dir;
        }

        if (!dir.isEmpty()) {
            explorer->setRootPath(dir);  // Load into your FileExplorer class
            settings.setValue("lastOpenedDirectory", dir);  // Save the new path
        }

        ui->left_widget->setVisible(true);
    }
    else if (!isEmpty && isVisible) {
        ui->left_widget->setVisible(false);
    }
    else {
        ui->left_widget->setVisible(true);
    }
    // qDebug() << "Settings file location: " << QSettings("MyCompany", "MyEditor").fileName();
}


// checking for the file tree view ar e empty  or not
bool MainWindow::isTreeViewEmpty(QTreeView *treeView)
{
    QAbstractItemModel *model = treeView->model();
    if (!model)
        return true;

    QModelIndex rootIndex = treeView->rootIndex();
    if (!rootIndex.isValid())
        rootIndex = model->index(0, 0);  // Fallback for some models

    return model->rowCount(rootIndex) == 0;
}

// terminal panel hide and show
void MainWindow::on_actionTerminal_triggered()
{    bool isVisible = ui->terminal_widget->isVisible();
    if(isVisible==true){
        ui->terminal_widget->setVisible(false);
    }else{
        ui->terminal_widget->setVisible(true);
        ui->terminal_tab->clear();
        QSettings settings("abc", "def");
        QString lastDir= settings.value("lastOpenedDirectory").toString();
        //TerminalWidget *terminal = new TerminalWidget(this, TerminalWidget::DefaultMode);
        terminal1->setInitialDirectory(lastDir);
        terminal1->startTerminal();
        int tabIndex=ui->terminal_tab->addTab(terminal1,QIcon(":/terminal.png"),"terminal");
        ui->terminal_tab->setCurrentIndex(tabIndex);
    }
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
    // CHighlighter *chighlighter = new CHighlighter(editor->document());
    //CppHighlighter *cpphighlighter = new CppHighlighter(editor->document());
}

// treeview expand and collapse

void MainWindow::on_minimize_structure_btn_clicked()
{
    QAbstractItemModel* model = ui->fileTreeView->model();
    if (!model) return;

    bool anyExpanded = false;

    // Recursively check if any node is expanded
    std::function<void(const QModelIndex&)> checkExpanded;
    checkExpanded = [&](const QModelIndex& parent) {
        int rowCount = model->rowCount(parent);
        for (int i = 0; i < rowCount; ++i) {
            QModelIndex index = model->index(i, 0, parent);
            if (ui->fileTreeView->isExpanded(index)) {
                anyExpanded = true;
                return;
            }
            checkExpanded(index); // Check children
        }
    };

    checkExpanded(QModelIndex());

    // Now expand or collapse all
    if (anyExpanded) {
        ui->fileTreeView->collapseAll();
        ui->minimize_structure_btn->setToolTip("Expand All");
    } else {
        ui->fileTreeView->expandAll();
        ui->minimize_structure_btn->setToolTip("Collapse All");
    }
}

// treeview hide or show

void MainWindow::on_hide_btn_clicked()
{
    bool isVisible = ui->left_widget->isVisible();
    if(isVisible){
        ui->left_widget->hide();
    }else{
        ui->left_widget->show();
    }
}



// customize the structure
void MainWindow::on_customize_combo_currentIndexChanged(int index)
{
    QFileSystemModel *currentModel = qobject_cast<QFileSystemModel*>(ui->fileTreeView->model());
    if (currentModel) {
        // Now you can use the currentModel, e.g., get the root directory
        QModelIndex rootIndex = ui->fileTreeView->rootIndex();
        QString currentPath = currentModel->filePath(rootIndex);
    //     qDebug() << "Currently opened directory:" << currentPath;
    // }

    // Modify the filter based on the selected option
    if (index == 4) {
        // All: Show both files and folders
        currentModel->setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
    } else if (index == 1) {
        // Files Only: Show only files
        currentModel->setFilter(QDir::Files | QDir::NoDotAndDotDot);
    } else if (index == 2) {
        // Folders Only: Show only folders
        currentModel->setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    }

    // Re-set the root directory to keep the same folder open after filtering
    ui->fileTreeView->setRootIndex(currentModel->index(currentPath));
    }
}



// new text file triggered...
void MainWindow::on_actionNew_Text_File_triggered()
{
    // create the instance of codeEditor class
    CodeEditor *editor = new CodeEditor(this);
    QIcon fileIcon=QIcon(":/txt.png");

    int tabIndex= ui->editor_tab->addTab(editor,fileIcon,"Untitled.txt");
    ui->editor_tab->setCurrentIndex(tabIndex);
    // Step 5: Track file path and type
    openFilePaths.insert(tabIndex, "");          // empty path means unsaved
    openFileExtensions.insert(tabIndex, "txt");    // store selected extension
    tabUnsavedMap[tabIndex] = false; // initially saved

    connect(editor, &QPlainTextEdit::textChanged, this, [=]() {
        markTabUnsaved(tabIndex);
    });
}

// save action triggered..
void MainWindow::on_actionSAve_triggered()
{
    int index = ui->editor_tab->currentIndex();
    if (index == -1) return;

    QPlainTextEdit *editor = qobject_cast<QPlainTextEdit *>(ui->editor_tab->widget(index));
    if (!editor) return;

    QString filePath = openFilePaths[index];

    if (filePath.isEmpty()) {
        // Not saved before → use Save As
        on_actionSave_As_triggered();
        onTextChanged(); // for clangD request
        markTabSaved(index);
        return;
    }

    QFile file(filePath);
    if (file.open(QFile::WriteOnly | QFile::Text)) {
        QTextStream out(&file);
        out << editor->toPlainText();
        file.close();
        editor->document()->setModified(false);
    } else {
        QMessageBox::warning(this, "Save Error", "Could not save file.");
    }

    ui->statusBar->showMessage("File Saved Successfully!!",2000);
    updateStatusBar(filePath);
    markTabSaved(index);
}

// Save as triggered..
void MainWindow::on_actionSave_As_triggered()
{
    int index = ui->editor_tab->currentIndex();
    if (index == -1) return;

    QPlainTextEdit *editor = qobject_cast<QPlainTextEdit *>(ui->editor_tab->widget(index));
    if (!editor) return;

    QString defaultExt = "txt"; // fallback if nothing found

    // Use stored extension from new-file logic
    if (index < openFileExtensions.size()) {
        defaultExt = openFileExtensions[index];
    } else if (index < openFilePaths.size() && !openFilePaths[index].isEmpty()) {
        // Use current file's extension if previously saved
        defaultExt = QFileInfo(openFilePaths[index]).suffix();
    }

    // Suggest file name with extension
    QString suggestedName = "untitled." + defaultExt;

    QString filePath = QFileDialog::getSaveFileName(
        this,
        "Save File As",
        suggestedName,
        "All Files (*)" // no filter selection needed
        );

    if (filePath.isEmpty()) return;

    // Auto-append extension if user didn't type one
    if (QFileInfo(filePath).suffix().isEmpty()) {
        filePath += "." + defaultExt;
    }

    // Save content
    QFile file(filePath);
    if (file.open(QFile::WriteOnly | QFile::Text)) {
        QTextStream out(&file);
        out << editor->toPlainText();
        file.close();
        editor->document()->setModified(false);

        // Update tracked paths and extension
        if (index >= openFilePaths.size()) openFilePaths.resize(index + 1);
        openFilePaths[index] = filePath;

        if (index >= openFileExtensions.size()) openFileExtensions.resize(index + 1);
        openFileExtensions[index] = QFileInfo(filePath).suffix();

        ui->editor_tab->setTabText(index, QFileInfo(filePath).fileName());
        statusBar()->showMessage("Saved as " + QFileInfo(filePath).fileName());
        onTextChanged(); // for clangD request
        updateStatusBar(filePath);
    } else {
        QMessageBox::warning(this, "Save Error", "Could not save file.");
    }
}

// new File triggered
/*
void MainWindow::on_actionNew_File_triggered()
{
    // Step 1: Ask user to select file type
    QStringList items = { "C File (.c)", "C++ File (.cpp)", "Header File (.h)", "Python File (.py)", "Text File (.txt)" };
    bool ok;
    QString choice = QInputDialog::getItem(this, "Select File Type", "Choose a file type:", items, 0, false, &ok);

    if (!ok || choice.isEmpty()) return;

    // Step 2: Map choice to extension
    QString ext;
    QIcon languageIcon;
    if (choice.contains(".cpp"))
    {
        ext = "cpp";
        languageIcon=QIcon(":/c++.png");
    }
    else if (choice.contains(".c")) {
        ext = "c";
        languageIcon=QIcon(":/c.png");
    }
    else if (choice.contains(".h")){
        ext = "h";
        languageIcon=QIcon(":/c++.png");
    }

    else if (choice.contains(".py")) {
        ext = "py";
        languageIcon=QIcon(":/python.png");
    }

    else {
        ext = "txt";
        languageIcon=QIcon(":/txt.png");
    }
    // Step 3: Create editor
    editor = new CodeEditor(this);

    // Step 4: Name the tab (Untitled.c, Untitled1.cpp, etc.)
    static int untitledCount = 1;
    QString tabName = "Untitled" + QString::number(untitledCount++) + "." + ext;

    int index= ui->editor_tab->addTab(editor,languageIcon,tabName);
    ui->editor_tab->setCurrentIndex(index);
    // Based On extension highlighter are applied..
    if(ext=="c"){
        // apply c highlighting
        new CHighlighter(editor->document());
    }else if(ext=="cpp" || ext=="h"){
        //apply c++ highlighter
        new CppHighlighter(editor->document());
    }else if(ext=="py"){
        // apply py highlighter
        new PythonHighlighter(editor->document());
    }
    // Step 5: Track file path and type
    openFilePaths.insert(index, "");          // empty path means unsaved
    openFileExtensions.insert(index, ext);    // store selected extension


    tabUnsavedMap[index] = false; // initially saved

    connect(editor, &QPlainTextEdit::textChanged, this, [=]() {
        markTabUnsaved(index);
    });

    ui->statusBar->showMessage("New File is Created!!!",3000);
    ui->statusBar->clearMessage();
    ui->statusBar->showMessage("File is not saved !!!!");
}
*/
void MainWindow::on_actionNew_File_triggered()
{
    // Step 1: Ask user to select file type
    QStringList items = { "C File (.c)", "C++ File (.cpp)", "Header File (.h)", "Python File (.py)", "Text File (.txt)" };
    bool ok;
    QString choice = QInputDialog::getItem(this, "Select File Type", "Choose a file type:", items, 0, false, &ok);

    if (!ok || choice.isEmpty()) return;

    // Step 2: Map choice to extension and icon
    QString ext;
    QIcon languageIcon;

    if (choice.contains(".cpp")) {
        ext = "cpp";
        languageIcon = QIcon(":/c++.png");
    } else if (choice.contains(".c")) {
        ext = "c";
        languageIcon = QIcon(":/c.png");
    } else if (choice.contains(".h")) {
        ext = "h";
        languageIcon = QIcon(":/c++.png");
    } else if (choice.contains(".py")) {
        ext = "py";
        languageIcon = QIcon(":/python.png");
    } else {
        ext = "txt";
        languageIcon = QIcon(":/txt.png");
    }

    // Step 3: Create a new editor
    CodeEditor *newEditor = new CodeEditor(this);

    // Step 4: Generate a unique untitled filename
    static int untitledCount = 1;
    QString tabName = "Untitled" + QString::number(untitledCount++) + "." + ext;

    // Step 5: Add to tab
    int index = ui->editor_tab->addTab(newEditor, languageIcon, tabName);
    ui->editor_tab->setCurrentIndex(index);

    // Step 6: Apply syntax highlighter
    if (ext == "c") {
        new CHighlighter(newEditor->document());
    } else if (ext == "cpp" || ext == "h") {
        new CppHighlighter(newEditor->document());
    } else if (ext == "py") {
        new PythonHighlighter(newEditor->document());
    }

    // Step 7: Generate a temp file URI (for LSP)
    QString tempFilePath = QDir::temp().filePath(tabName);
    newEditor->setProperty("filePath", tempFilePath);
    filePaths = tempFilePath; // update globally if needed

    // Step 8: Store in tab maps
    openFilePaths.insert(index, ""); // empty path = unsaved
    openFileExtensions.insert(index, ext);
    tabUnsavedMap[index] = false;
    editor = newEditor;

    // Step 9: Connect textChanged to update diagnostics
    connect(newEditor, &QPlainTextEdit::textChanged, this, [=]() {
        markTabUnsaved(index);
        if (diagnosticsTimer) {
            diagnosticsTimer->start(); // debounce timer for didChange
        }
    });

    // Step 10: Send didOpen + symbol request to clangd
    if (lspClient) {
        QString content = newEditor->toPlainText(); // likely empty
        QString uri = QUrl::fromLocalFile(tempFilePath).toString();

        lspClient->sendDidOpen(uri, ext, content);          // Notify LSP
        lspClient->requestDocumentSymbols(uri);             // Load symbols
    }

    // Step 11: Show status messages
    ui->statusBar->showMessage("New file created!", 3000);
    QTimer::singleShot(3000, this, [=]() {
        ui->statusBar->showMessage("File is not saved!");
    });
}

// New Package triggered
void MainWindow::on_actionNew_Package_triggered()
{

    // Step 1: Ask for package name
    QString packageName = QInputDialog::getText(this, "New Package", "Enter package name:");
    if (packageName.isEmpty()) return;

    // Step 2: Ask user to select directory
    QString baseDir = QFileDialog::getExistingDirectory(this, "Select location to create package");
    if (baseDir.isEmpty()) return;

    // Step 3: Create full package path
    QDir dir(baseDir);
    if (!dir.mkdir(packageName)) {
        QMessageBox::warning(this, "Error", "Failed to create package directory.");
        return;
    }

    QString packagePath = dir.filePath(packageName);

    // qDebug()<<packageName <<"and"<<packagePath;

    QString parentPath = QFileInfo(packagePath).absolutePath();

    // Open this package in treview
    // setUpTreeView(parentPath);
    ui->fileTreeView->setRootIndex(model->index(parentPath));

    // set it is last opened
    QSettings settings("abc", "def");
    settings.setValue("lastOpenedDirectory", parentPath);


    ui->statusBar->showMessage("New Package is created",3000);
}


void MainWindow::on_actionOpen_File_triggered() {
    QString filePath = QFileDialog::getOpenFileName(this, "Open File", "", "All Files (*.*)");
    if (filePath.isEmpty())
        return;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Cannot open file:\n" + file.errorString());
        return;
    }

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    // Create text editor tab
    CodeEditor *editor = new CodeEditor(this);
    editor->setPlainText(content);


    // Based on extension file icon and highlighter apply
    QString suffix = QFileInfo(filePath).suffix().toLower();
    QIcon fileIcon;
    if (suffix == "cpp" || suffix == "h" || suffix == "hpp") {
        fileIcon = QIcon(":/c++.png"); // C++ icon
        new CppHighlighter(editor->document());
    } else if (suffix == "py") {
        fileIcon = QIcon(":/python.png"); // Python icon
    } else if (suffix == "txt") {
        fileIcon = QIcon(":/txt.png"); // Text file icon
    } else if (suffix == "c") {
        fileIcon = QIcon(":/c.png"); // c file icon
        // call c syntax highlighting
        new CHighlighter(editor->document());
    }
    int index = ui->editor_tab->addTab(editor,fileIcon, QFileInfo(filePath).fileName());
    ui->editor_tab->setCurrentIndex(index);


    // Track file path and extension
    if (index >= openFilePaths.size()) openFilePaths.resize(index + 1);
    openFilePaths[index] = filePath;

    if (index >= openFileExtensions.size()) openFileExtensions.resize(index + 1);
    openFileExtensions[index] = QFileInfo(filePath).suffix();

    updateStatusBar(filePath);
}



void MainWindow::on_actionOpen_Folder_triggered()
{
    // Step 1: Ask user to select folder
    QString folderPath = QFileDialog::getExistingDirectory(this, "Open Package Folder");
    if (folderPath.isEmpty()) return;


    // setup left widget view
    setUpTreeView(folderPath);
    // ui->fileTreeView->setRootIndex(model->index(folderPath));

    QSettings settings("abc", "def");
    settings.setValue("lastOpenedDirectory", folderPath);

    // add recently opened
    updateRecentFoldersComboBox(folderPath);
}

void MainWindow::updateRecentFoldersComboBox(const QString &newPath) {
    recentFolders.removeAll(newPath);         // Remove duplicates
    recentFolders.prepend(newPath);           // Add to top
    while (recentFolders.size() > 10)         // Keep max 10
        recentFolders.removeLast();

    // Update combo box
    ui->recently_opened_combo->clear();
    ui->recently_opened_combo->addItems(recentFolders);

    // Save to QSettings
    QSettings settings("abc", "def");
    settings.setValue("recentFolders", recentFolders);
}



void MainWindow::on_recently_opened_combo_activated(int index)
{
    QString path = ui->recently_opened_combo->itemText(index);
    if (!QDir(path).exists()) {
        QMessageBox::warning(this, "Folder Not Found", "This folder no longer exists.");
        return;
    }
    ui->terminal_tab->clear();
    ui->terminal_widget->setVisible(false);
    setUpTreeView(path);
    QSettings settings("abc", "def");
    settings.setValue("lastOpenedDirectory", path);
    // ui->fileTreeView->setRootIndex(model->index(path));
}

// show history - last
void MainWindow::on_actionShow_History_triggered() {
    // Load recentFolders from QSettings
    QSettings settings("abc", "def");
    QStringList history = settings.value("recentFolders").toStringList();

    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("Recent Folder History");
    dialog->setFixedSize(500, 400);

    QVBoxLayout *layout = new QVBoxLayout(dialog);
    QListWidget *listWidget = new QListWidget(dialog);

    listWidget->addItems(history.mid(0, 20)); // last 20
    listWidget->setStyleSheet(R"(
        QListWidget {
            color: white;
            font-size: 14px;
            border: none;
            background-color: #2c2c2c;
        }
        QListWidget::item {
            padding: 8px;
        }
        QListWidget::item:hover {
            background-color: #1a73e8;
        }
    )");

    layout->addWidget(listWidget);
    dialog->setLayout(layout);

    // 📂 Handle double-click to open folder
    connect(listWidget, &QListWidget::itemDoubleClicked, this, [=](QListWidgetItem *item) {
        QString path = item->text();
        if (!QDir(path).exists()) {
            QMessageBox::warning(dialog, "Folder Not Found", "This folder no longer exists.");
            return;
        }

        // Set treeView root
        setUpTreeView(path);
        // ui->fileTreeView->setRootIndex(model->index(path));

        // Update recent combo box and move folder to top
        updateRecentFoldersComboBox(path);

        dialog->accept(); // close dialog
    });

    dialog->exec();
}

void MainWindow::on_actionSave_All_triggered()
{
    for (int i = 0; i < ui->editor_tab->count(); ++i) {
        QWidget *tab = ui->editor_tab->widget(i);
        QPlainTextEdit *editor = tab->findChild<QPlainTextEdit*>();

        if (!editor)
            continue;

        QString filePath = openFilePaths[i];
        if (!editor->document()->isModified())
            continue;

        if (filePath.isEmpty()) {
            // No file path yet, ask for Save As
            QString suggestedExt = tab->property("fileExtension").toString(); // e.g. ".cpp"
            QString newPath = QFileDialog::getSaveFileName(
                this,
                "Save As",
                QDir::homePath() + "/Untitled" + suggestedExt,
                QString("*.%1").arg(suggestedExt.mid(1)) // removes dot
                );
            if (newPath.isEmpty())
                continue;

            filePath = newPath;
            openFilePaths[i] = filePath;
            ui->editor_tab->setTabText(i, QFileInfo(filePath).fileName());
        }

        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << editor->toPlainText();
            file.close();
            editor->document()->setModified(false);
        } else {
            QMessageBox::warning(this, "Error", "Failed to save file: " + filePath);
        }
    }

    ui->statusBar->showMessage("Saved All..",3000);
}


void MainWindow::on_actionClose_Projects_triggered()
{
    // save all the tab widget
    on_actionSave_All_triggered();

    // all opened tab close
    ui->editor_tab->clear();
    ui->fileTreeView->setModel(nullptr);         // Detach the model
    ui->fileTreeView->setRootIndex(QModelIndex());  // Optional: reset view
    updateStatusBar("");
}


void MainWindow::on_actionExit_triggered() {
    QMessageBox::StandardButton reply;
    reply=QMessageBox::question(this,"Close Application","Do you want to close this application?",
                                  QMessageBox::Yes | QMessageBox::No);
    if(reply==QMessageBox::Yes){
        on_actionSave_All_triggered();
        QApplication::quit();
    }else{
        return;
    }
}

// helper Method

void MainWindow::updateFileContent(const QString &newContent) {
    // Send didChange request to Clangd
    QJsonObject didChangeRequest;
    didChangeRequest["jsonrpc"] = "2.0";
    didChangeRequest["method"] = "textDocument/didChange";
    didChangeRequest["params"] = QJsonObject{
        {"textDocument", QJsonObject{
                             {"uri", "file://" + filePaths},
                             {"version", 2}  // Increment version if necessary
                         }},
        {"contentChanges", QJsonArray{
                               QJsonObject{
                                   {"text", newContent}
                               }
                           }}
    };
}

void MainWindow::onTextChanged() {
    if (!editor || !lspClient) return;

    QString uri = QUrl::fromLocalFile(filePaths).toString();
    //qDebug()<< "URI is ::" << uri;
    QString content = editor->toPlainText();
    //qDebug()<< "Content is::"<<content;
    // Immediate minimal change notification
    lspClient->sendDidChange(uri, content);
    lspClient->requestDocumentSymbols(uri);
    // Start (or restart) timer for debounce diagnostics update
    diagnosticsTimer->start();
    // updateFileContent(newContent);
}

void MainWindow::on_terminal_hide_btn_clicked()
{
    ui->terminal_widget->setVisible(false);
}


void MainWindow::on_more_btn_clicked()
{
    if(ui->more_btn->isChecked()) {
    QPoint pos = ui->more_btn->mapToGlobal(QPoint(0, ui->more_btn->height()));
        popupWidget->move(pos);
        popupWidget->show();
        ui->more_btn->setIcon(QIcon(":/cross.png"));
    }else{
        popupWidget->hide();
        ui->more_btn->setIcon(QIcon(":/menu-burger.png"));
    }

}


// helper function
void MainWindow::markTabUnsaved(int index) {
    if (!tabUnsavedMap.value(index, false)) {
        QString currentText = ui->editor_tab->tabText(index);
        if (!currentText.startsWith("*")) {
            ui->editor_tab->setTabText(index, "*" + currentText);
        }
        tabUnsavedMap[index] = true;
    }
}


void MainWindow::markTabSaved(int index) {
    if (tabUnsavedMap.value(index, false)) {
        QString currentText = ui->editor_tab->tabText(index);
        if (currentText.startsWith("*")) {
            ui->editor_tab->setTabText(index, currentText.mid(1));  // remove *
        }
        tabUnsavedMap[index] = false;
    }
}

void MainWindow::closeAllTabsWithPrompt() {
    for (int i = 0; i < ui->editor_tab->count(); /* no increment here */) {
        if (tabUnsavedMap.value(i, false)) {
            QString tabTitle = ui->editor_tab->tabText(i);
            if (tabTitle.startsWith("*"))
                tabTitle = tabTitle.mid(1);  // remove asterisk for display

            QMessageBox::StandardButton reply = QMessageBox::question(
                this,
                "Unsaved Changes",
                QString("The file '%1' has unsaved changes. What would you like to do?").arg(tabTitle),
                QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
                QMessageBox::Save
                );

            if (reply == QMessageBox::Cancel) {
                return; // cancel whole operation
            } else if (reply == QMessageBox::Save) {
                // Simulate save
                // You can call your own saveTab(i) function
                on_actionSAve_triggered();
                //markTabSaved(i);
            }
        }

        // Remove the tab after save/discard
        tabUnsavedMap.remove(i);
        ui->editor_tab->removeTab(i);  // Do not increment 'i' since the tabs shift left
    }
}


void MainWindow::closeSavedTabs()
{
    // Work in reverse to avoid shifting indices when removing tabs
    for (int i = ui->editor_tab->count() - 1; i >= 0; --i) {
        if (tabUnsavedMap.contains(i) && tabUnsavedMap[i] == false) {
            QWidget *widget = ui->editor_tab->widget(i);
            ui->editor_tab->removeTab(i);
            delete widget;

            // Clean up maps
            tabUnsavedMap.remove(i);
            openFilePaths.remove(i);
            openFileExtensions.remove(i);
        }
    }
}

void MainWindow::toggleLockTab()
{
    int index = ui->editor_tab->currentIndex();
    if (index < 0) return;

    QWidget *widget = ui->editor_tab->widget(index);
    if (!widget) return;

    CodeEditor *editor = qobject_cast<CodeEditor *>(widget);
    if (!editor) return;

    bool isLocked = tabLockedMap.value(index, false);
    isLocked = !isLocked;  // Toggle

    editor->setReadOnly(isLocked);
    tabLockedMap[index] = isLocked;

    // 🌟 Optionally update tab text with [🔒] or normal
    QString tabText = ui->editor_tab->tabText(index);
    if (isLocked && !tabText.startsWith("🔒 ")) {
        ui->editor_tab->setTabText(index, "🔒 " + tabText);
    } else if (!isLocked && tabText.startsWith("🔒 ")) {
        ui->editor_tab->setTabText(index, tabText.mid(3)); // remove lock icon
    }

    // 🌟 Optionally change tooltip or background color if desired
}


// get the current Code Editor object
CodeEditor* MainWindow::currentEditor() const {
    return qobject_cast<CodeEditor *>(ui->editor_tab->currentWidget());
}


void MainWindow::on_actionUndo_triggered()
{
    if (auto editor = currentEditor()) editor->undo();
}


void MainWindow::on_actionRedo_triggered()
{
    if (auto editor = currentEditor()) editor->redo();
}


void MainWindow::on_actionCut_triggered()
{
    if (auto editor = currentEditor()) editor->cut();
}


void MainWindow::on_actionCopy_triggered()
{
    if (auto editor = currentEditor()) editor->copy();
}


void MainWindow::on_actionPaste_triggered()
{
    if (auto editor = currentEditor()) editor->paste();
}


void MainWindow::on_actionSelect_All_triggered()
{
    if (auto editor = currentEditor()) editor->selectAll();
}


void MainWindow::on_actionSearch_triggered()
{

}


void MainWindow::on_actionProblems_triggered()
{
    // check the terminal widget are show or
    if(!ui->terminal_widget->isVisible()){
        // visible
        ui->terminal_widget->setVisible(true);
        setupProblemsList();
    }        // then add the problem tab
    int tabIndex=ui->terminal_tab->addTab(problemsTable,QIcon(":/issue.png"),"problems");
    ui->terminal_tab->setCurrentIndex(tabIndex);
}

int findTabIndexByName(QTabWidget *tabWidget, const QString &tabName) {
    for (int i = 0; i < tabWidget->count(); ++i) {
        if (tabWidget->tabText(i) == tabName)
            return i;  // Found
    }
    return -1;  // Not found
}




void MainWindow::on_problems_btn_clicked()
{
    // check the problems tab are present or not

    int tabIndex= findTabIndexByName(ui->terminal_tab,"problems");

    if(tabIndex!=-1){
        // found -- allocating
        ui->terminal_tab->setCurrentIndex(tabIndex);
    }else{
        //setupProblemsList();
        ui->terminal_tab->addTab(problemsTable,QIcon(":/issue.png"),"problems");
        ui->terminal_tab->setCurrentIndex(ui->terminal_tab->count() - 1);
    }
}


void MainWindow::on_terminal_btn_clicked()
{
    int tabIndex= findTabIndexByName(ui->terminal_tab,"terminal");
    if(tabIndex!=-1){
        ui->terminal_tab->setCurrentIndex(tabIndex);
    }else{
        QSettings settings("abc", "def");
        QString lastDir= settings.value("lastOpenedDirectory").toString();
        //TerminalWidget *terminal = new TerminalWidget(this, TerminalWidget::DefaultMode);
        terminal1->setInitialDirectory(lastDir);
        terminal1->startTerminal();
        int tabIndex=ui->terminal_tab->addTab(terminal1,QIcon(":/terminal.png"),"terminal");
        ui->terminal_tab->setCurrentIndex(tabIndex);
    }
    terminal1->setReadOnly(false);
}

// symbol table related code

QString kindToCategory(int kind) {
    switch (kind) {
    case 5: return "Class";
    case 6: return "Method";
    case 12: return "Function";
    case 13: return "Variable";
    case 14: return "Constant";
    default: return "Symbol";
    }
}
struct LocalVariable {
    QString type;
    QString name;
    int line;
};

QList<LocalVariable> extractLocalVariablesWithLines(const QTextBlock &startBlock, int functionStartLine) {
    QList<LocalVariable> locals;
    QTextBlock block = startBlock;

    int openBraces = 0;
    bool started = false;
    int line = functionStartLine;

    // Match: int *a, b[10], c;
    QRegularExpression declarationRegex(R"(\b(int|float|double|char|long|short)\s+((?:[^;])+))");

    // Match individual vars: *ptr, arr[10], var
    QRegularExpression varRegex(R"(\*?\s*([a-zA-Z_]\w*)(\[[^\]]*\])?)");

    while (block.isValid()) {
        QString text = block.text();

        openBraces += text.count('{');
        openBraces -= text.count('}');

        if (text.contains("{")) started = true;
        if (started && openBraces <= 0) break;

        auto declMatchIter = declarationRegex.globalMatch(text);
        while (declMatchIter.hasNext()) {
            auto declMatch = declMatchIter.next();
            QString baseType = declMatch.captured(1);
            QString varList = declMatch.captured(2);

            for (const QString &varRaw : varList.split(',', Qt::SkipEmptyParts)) {
                QString var = varRaw.trimmed();
                auto varMatch = varRegex.match(var);
                if (varMatch.hasMatch()) {
                    QString varName = varMatch.captured(1);
                    QString arraySuffix = varMatch.captured(2);
                    QString pointer = var.contains('*') ? "*" : "";

                    QString fullType = baseType;
                    if (!pointer.isEmpty()) fullType += " *";
                    if (!arraySuffix.isEmpty()) fullType += " " + arraySuffix;

                    locals.append({fullType.trimmed(), varName, line + 1});
                }
            }
        }

        block = block.next();
        ++line;
    }

    return locals;
}


void MainWindow::populateSymbolTable(const QJsonArray &symbols) {
    symbolTableModel->removeRows(0, symbolTableModel->rowCount());
    QSet<QString> uniqueKeys;
    parseAndAddSymbols(symbols, "Global", uniqueKeys);
}

void MainWindow::parseAndAddSymbols(const QJsonArray &symbols, const QString &parentScope, QSet<QString> &uniqueKeys){

    for (const QJsonValue &val : symbols) {
        QJsonObject obj = val.toObject();

        QString name = obj["name"].toString();
        QString type = obj["detail"].toString();
        int kind = obj["kind"].toInt();
        QString category = kindToCategory(kind);
        QString scope = parentScope;

        QJsonObject start = obj["range"].toObject()["start"].toObject();
        int line = start["line"].toInt();

        QList<QStandardItem *> row;
        QStandardItem *lineItem = new QStandardItem(QString::number(line + 1));
        lineItem->setData(line, Qt::UserRole);

        row << lineItem
            << new QStandardItem(name)
            << new QStandardItem(type)
            << new QStandardItem(scope)
            << new QStandardItem(category);
        QString key = name ;
        if (uniqueKeys.contains(key))
            continue; // ❌ Skip duplicate symbol
        uniqueKeys.insert(key);

        symbolTableModel->appendRow(row);

        // ✅ Check and recurse
        if (obj.contains("children") && obj["children"].isArray()) {
            QJsonArray children = obj["children"].toArray();
            //qDebug() << "🔁 Recursing into" << name << "with" << children.size() << "children";
            parseAndAddSymbols(children, name,uniqueKeys);
        }
        // ✅ NEW: If it's a function, parse local variables via regex
        // ✅ NEW: If it's a function, parse local variables via regex
        if (category == "Function" && currentEditor()) {
            QTextDocument *doc = currentEditor()->document();
            QTextBlock startBlock = doc->findBlockByNumber(line);

            auto locals = extractLocalVariablesWithLines(startBlock, line);
            for (const LocalVariable &var : locals) {
                QString key = var.name ;
                if (uniqueKeys.contains(key))
                    continue; // ❌ Already added
                uniqueKeys.insert(key);

                QList<QStandardItem *> localRow;
                QStandardItem *lineItem = new QStandardItem(QString::number(var.line));
                lineItem->setData(var.line - 1, Qt::UserRole);

                localRow << lineItem
                         << new QStandardItem(var.name)
                         << new QStandardItem(var.type)
                         << new QStandardItem("Local")
                         << new QStandardItem("Variable");

                symbolTableModel->appendRow(localRow);
            }

        }
    }
}



void MainWindow::on_actionNew_Projects_triggered()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Select Location");
    if (dir.isEmpty())
        return;

    bool ok;
    QString name = QInputDialog::getText(this, "Project Name", "Enter new project name:", QLineEdit::Normal, "", &ok);
    if (!ok || name.isEmpty())
        return;

    QString path = dir + "/" + name;
    QDir projectDir(path);
    if (projectDir.exists()) {
        QMessageBox::warning(this, "Error", "Project already exists.");
        return;
    }
    createProjectMarker(path,name,"cpp");

    // Create folders
    projectDir.mkpath("src");
    projectDir.mkpath("include");

    // Create main.cpp
    QFile mainFile(path + "/src/main.cpp");
    if (mainFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&mainFile);
        out << "#include <iostream>\n";
        out<< "using namespace std;\n\n";
        out << "int main() {\n    std::cout << \"Hello from " << name << "\" << std::endl;\n    return 0;\n}";
        mainFile.close();
    }

    // // Optional .pro file
    // QFile proFile(path + "/" + name + ".pro");
    // if (proFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
    //     QTextStream out(&proFile);
    //     out << "TEMPLATE = app\nCONFIG += console c++17\n";
    //     out << "SOURCES += src/main.cpp\nINCLUDEPATH += include\n";
    //     proFile.close();
    // }

    // Update project model view
    currentProjectPath = path;


    setUpTreeView(currentProjectPath);
    // ui->fileTreeView->setRootIndex(model->index(folderPath))

    // add recently opened
    updateRecentFoldersComboBox(currentProjectPath);

    saveToRecentProjects(currentProjectPath);
}

void MainWindow::createProjectMarker(const QString &projectPath, const QString &projectName, const QString &language) {
    QDir dir(projectPath);
    if (!dir.exists())
        dir.mkpath(".");

    QFile file(dir.filePath("myideproject.json"));
    if (file.open(QIODevice::WriteOnly)) {
        QJsonObject obj;
        obj["project_name"] = projectName;
        obj["created_by"] = "MyCodeEditor";
        obj["version"] = "1.0";
        obj["language"] = language;
        obj["last_opened_file"] = "";

        QJsonDocument doc(obj);
        file.write(doc.toJson(QJsonDocument::Indented));
        file.close();
        //qDebug() << "Project marker created at:" << file.fileName();
    } else {
        qWarning() << "Failed to create marker at:" << file.fileName();
    }
}



void MainWindow::on_actionOpen_Projects_triggered()
{
    QString dirPath = QFileDialog::getExistingDirectory(this, "Open Project Folder");

    if (dirPath.isEmpty())
        return;

    QDir projectDir(dirPath);
    QString markerPath = projectDir.filePath("myideproject.json");

    QFile markerFile(markerPath);
    if (!markerFile.exists()) {
        QMessageBox::warning(this, "Invalid Project", "This folder is not a valid MyCodeEditor project.");
        return;
    }

    if (!markerFile.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "Error", "Cannot open project file: " + markerPath);
        return;
    }

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(markerFile.readAll(), &error);
    markerFile.close();

    if (error.error != QJsonParseError::NoError || !doc.isObject()) {
        QMessageBox::critical(this, "Error", "Project file is corrupted or invalid.");
        return;
    }

    QJsonObject obj = doc.object();
    QString projectName = obj.value("project_name").toString();
    QString language = obj.value("language").toString();

    //qDebug() << "Opened project:" << projectName << " (" << language << ")";

    currentProjectPath = dirPath;


    setUpTreeView(currentProjectPath);
    // ui->fileTreeView->setRootIndex(model->index(folderPath));


    // add recently opened
    updateRecentFoldersComboBox(currentProjectPath);

    saveToRecentProjects(currentProjectPath);
}

void MainWindow::saveToRecentProjects(const QString &projectPath) {
    QSettings settings("MyCodeEditor", "ProjectManager");

    QStringList recent = settings.value("recentProjects").toStringList();

    // Remove duplicates
    recent.removeAll(projectPath);
    recent.prepend(projectPath); // Newest on top

    // Optional: Limit to last 10 projects
    while (recent.size() > 10)
        recent.removeLast();

    settings.setValue("recentProjects", recent);
}



void MainWindow::on_actionRecent_Projects_triggered()
{
    QSettings settings("MyCodeEditor", "ProjectManager");
    QStringList history = settings.value("recentProjects").toStringList();

    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("Recent Folder History");
    dialog->setFixedSize(500, 400);

    QVBoxLayout *layout = new QVBoxLayout(dialog);
    QListWidget *listWidget = new QListWidget(dialog);

    listWidget->addItems(history.mid(0, 20)); // last 20
    listWidget->setStyleSheet(R"(
        QListWidget {
            color: white;
            font-size: 14px;
            border: none;
            background-color: #2c2c2c;
        }
        QListWidget::item {
            padding: 8px;
        }
        QListWidget::item:hover {
            background-color: #1a73e8;
        }
    )");

    layout->addWidget(listWidget);
    dialog->setLayout(layout);

    // 📂 Handle double-click to open folder
    connect(listWidget, &QListWidget::itemDoubleClicked, this, [=](QListWidgetItem *item) {
        QString path = item->text();
        if (!QDir(path).exists()) {
            QMessageBox::warning(dialog, "Folder Not Found", "This folder no longer exists.");
            return;
        }

        // Set treeView root
        setUpTreeView(path);
        // ui->fileTreeView->setRootIndex(model->index(path));

        // Update recent combo box and move folder to top
        updateRecentFoldersComboBox(path);

        dialog->accept(); // close dialog
    });

    dialog->exec();
}


void MainWindow::on_actionClose_Projects_2_triggered()
{
    // save all the tab widget
    on_actionSave_All_triggered();

    // // all opened tab close
    // ui->editor_tab->clear();
    // ui->fileTreeView->setModel(nullptr);         // Detach the model
    // ui->fileTreeView->setRootIndex(QModelIndex());  // Optional: reset view
    // updateStatusBar("");

    if (!ui->fileTreeView->isEnabled()) {
        QMessageBox::information(this, "No Project Open", "There is no project to close.");
        return;
    }

    // Optional: ask for confirmation
    if (QMessageBox::question(this, "Close Project",
                              "Are you sure you want to close the current project?") != QMessageBox::Yes)
        return;

    // Step 1: Clear the file tree view
    ui->fileTreeView->setModel(nullptr);

    // Step 2: Clear editor tabs if you have any (example shown)
    ui->editor_tab->clear(); // or close all open documents

    // Step 3: Reset current project path
    currentProjectPath.clear();

    // Step 4: Update status bar or UI elements
    ui->statusBar->showMessage("Project closed");
}


void MainWindow::on_actionRename_Symbols_triggered()
{
    CodeEditor *editor = currentEditor(); // custom method to get active editor
    if (!editor)
        return;

    int line = editor->textCursor().blockNumber();
    int column = editor->textCursor().positionInBlock();

    QString filePath = editor->currentFilePath();
    if (filePath.isEmpty()) return;



    bool ok;
    QString newName = QInputDialog::getText(this, "Rename Symbol", "New name:", QLineEdit::Normal, "", &ok);
    if (!ok || newName.trimmed().isEmpty()) return;

    if (lspClient)
        lspClient->renameSymbol(filePaths, line, column, newName);
}

void MainWindow::applyRenameEdits(const QJsonObject &changes) {
    for (const QString &uri : changes.keys()) {
        CodeEditor *editor = getEditorByUri(uri); // your method to get editor by file
        if (!editor) continue;

        QJsonArray edits = changes[uri].toArray();
        QTextCursor cursor = editor->textCursor();
        cursor.beginEditBlock();

        for (const QJsonValue &val : edits) {
            QJsonObject edit = val.toObject();
            QJsonObject range = edit["range"].toObject();
            QJsonObject start = range["start"].toObject();
            QJsonObject end = range["end"].toObject();

            int startLine = start["line"].toInt();
            int startChar = start["character"].toInt();
            int endLine = end["line"].toInt();
            int endChar = end["character"].toInt();
            QString newText = edit["newText"].toString();

            QTextCursor c = editor->textCursorForRange(startLine, startChar, endLine, endChar);
            c.removeSelectedText();
            c.insertText(newText);
        }

        cursor.endEditBlock();
    }
}
CodeEditor* MainWindow::getEditorByUri(const QString &uri) {
    QString path = QUrl::fromPercentEncoding(uri.toUtf8().mid(8)); // remove file:///
    return openEditors.contains(path) ? openEditors[path] : nullptr;
}


void MainWindow::on_actionChange_All_Occurances_triggered()
{
    CodeEditor *editor = currentEditor();  // your method to get the active editor
    if (!editor) return;

    QString word = editor->getSymbolUnderCursor();
    if (word.isEmpty()) {
        QMessageBox::warning(this, "Change All Occurrences", "No word selected.");
        return;
    }

    QString newWord = QInputDialog::getText(this, "Change All Occurrences",
                                            QString("Change all occurrences of '%1' to:").arg(word),
                                            QLineEdit::Normal, word);
    if (newWord.isEmpty() || newWord == word) return;

    int count = editor->changeAllOccurrences(word, newWord);
    QMessageBox::information(this, "Done", QString("Replaced %1 occurrences.").arg(count));
}


void MainWindow::on_actionRename_Filename_triggered()
{
    CodeEditor *editor = currentEditor(); // Your method to get active tab editor
    if (!editor) return;

    QString oldPath = editor->currentFilePath(); // Full absolute path
    if (oldPath.isEmpty()) return;

    QString oldName = QFileInfo(oldPath).fileName();

    bool ok;
    QString newName = QInputDialog::getText(this, "Rename File", "New filename:", QLineEdit::Normal, oldName, &ok);
    if (!ok || newName.trimmed().isEmpty() || newName == oldName) return;

    QString dir = QFileInfo(oldPath).absolutePath();
    QString newPath = dir + "/" + newName;

    if (QFile::exists(newPath)) {
        QMessageBox::warning(this, "File Exists", "A file with that name already exists.");
        return;
    }

    if (!QFile::rename(oldPath, newPath)) {
        QMessageBox::critical(this, "Rename Failed", "Could not rename the file.");
        return;
    }

    editor->setCurrentFilePath(newPath);         // Update path in editor
    int index = ui->editor_tab->indexOf(editor);      // Update tab title
    if (index != -1) ui->editor_tab->setTabText(index, newName);

    openFilePaths.insert(index,newPath);
    QMessageBox::information(this, "Success", "File renamed successfully.");
}


void MainWindow::on_actionClose_Tab_triggered()
{
    // close the current tab
    int currentIndex = ui->editor_tab->currentIndex();
    if (currentIndex == -1) {
        QMessageBox::information(this, "No Tab", "No tab is currently open.");
        return;
    }

    CodeEditor *editor = currentEditor();
    if (!editor) return;

    // Check for unsaved changes
    if (editor->document()->isModified()) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            "Unsaved Changes",
            "This file has unsaved changes. Do you want to close without saving?",
            QMessageBox::Yes | QMessageBox::No
            );
        if (reply == QMessageBox::No)
            return; // Abort closing
    }

    // Remove from openEditors map if it's an opened file
    QString filePath = editor->currentFilePath();
    if (!filePath.isEmpty() && openEditors.contains(filePath)) {
        openEditors.remove(filePath);
    }

    // Remove tab and delete widget
    QWidget *tab = ui->editor_tab->widget(currentIndex);
    ui->editor_tab->removeTab(currentIndex);
    tab->deleteLater();
}


void MainWindow::on_actionClose_Other_Tabs_triggered()
{
    int currentIndex = ui->editor_tab->currentIndex();
    if (currentIndex == -1) {
        QMessageBox::information(this, "No Tab", "No tab is currently open.");
        return;
    }

    QWidget *currentWidget = ui->editor_tab->widget(currentIndex);
    QList<int> tabsToClose;

    // Collect indexes of tabs to close (excluding current)
    for (int i = 0; i < ui->editor_tab->count(); ++i) {
        if (i != currentIndex) {
            tabsToClose.append(i);
        }
    }

    // Reverse the list to avoid index shifting when removing tabs
    std::sort(tabsToClose.begin(), tabsToClose.end(), std::greater<int>());

    for (int i : tabsToClose) {
        QWidget *tab = ui->editor_tab->widget(i);
        CodeEditor *editor = qobject_cast<CodeEditor *>(tab);
        if (!editor) continue;

        // Check unsaved changes
        if (editor->document()->isModified()) {
            QMessageBox::StandardButton reply = QMessageBox::question(
                this,
                "Unsaved Changes",
                QString("Tab '%1' has unsaved changes. Close it?").arg(ui->editor_tab->tabText(i)),
                QMessageBox::Yes | QMessageBox::No
                );
            if (reply == QMessageBox::No)
                continue;
        }

        // Remove from openEditors if it's a file
        QString path = editor->currentFilePath();
        if (!path.isEmpty()) {
            openEditors.remove(path);
        }

        ui->editor_tab->removeTab(i);
        tab->deleteLater();
    }
}


void MainWindow::on_actionClose_All_Tabs_triggered()
{
    if (ui->editor_tab->count() == 0) {
        QMessageBox::information(this, "No Tabs", "There are no open tabs to close.");
        return;
    }

    // Iterate in reverse to avoid index shifting
    for (int i = ui->editor_tab->count() - 1; i >= 0; --i) {
        QWidget *tab = ui->editor_tab->widget(i);
        CodeEditor *editor = qobject_cast<CodeEditor *>(tab);
        if (!editor) continue;

        // Check for unsaved changes
        if (editor->document()->isModified()) {
            QMessageBox::StandardButton reply = QMessageBox::question(
                this,
                "Unsaved Changes",
                QString("Tab '%1' has unsaved changes. Close it?").arg(ui->editor_tab->tabText(i)),
                QMessageBox::Yes | QMessageBox::No
                );
            if (reply == QMessageBox::No)
                continue;
        }

        // Remove from openEditors
        QString path = editor->currentFilePath();
        if (!path.isEmpty()) {
            openEditors.remove(path);
        }

        ui->editor_tab->removeTab(i);
        tab->deleteLater();
    }
}


void MainWindow::on_actionClose_Tabs_To_The_Left_triggered()
{
    int currentIndex = ui->editor_tab->currentIndex();

    if (currentIndex <= 0) {
        QMessageBox::information(this, "No Tabs to Close", "There are no tabs to the left of the current tab.");
        return;
    }

    for (int i = currentIndex - 1; i >= 0; --i) {
        QWidget *tab = ui->editor_tab->widget(i);
        CodeEditor *editor = qobject_cast<CodeEditor *>(tab);
        if (!editor) continue;

        // Prompt for unsaved changes
        if (editor->document()->isModified()) {
            QMessageBox::StandardButton reply = QMessageBox::question(
                this,
                "Unsaved Changes",
                QString("Tab '%1' has unsaved changes. Close it?").arg(ui->editor_tab->tabText(i)),
                QMessageBox::Yes | QMessageBox::No
                );
            if (reply == QMessageBox::No)
                continue;
        }

        // Remove from map
        QString path = editor->currentFilePath();
        if (!path.isEmpty())
            openEditors.remove(path);

        ui->editor_tab->removeTab(i);
        tab->deleteLater();
    }
}


void MainWindow::on_actionClose_Tabs_To_The_Right_triggered()
{
    int currentIndex = ui->editor_tab->currentIndex();
    int totalTabs = ui->editor_tab->count();

    if (currentIndex >= totalTabs - 1) {
        QMessageBox::information(this, "No Tabs to Close", "There are no tabs to the right of the current tab.");
        return;
    }

    // Close tabs from last to currentIndex+1 to avoid shifting
    for (int i = totalTabs - 1; i > currentIndex; --i) {
        QWidget *tab = ui->editor_tab->widget(i);
        CodeEditor *editor = qobject_cast<CodeEditor *>(tab);
        if (!editor) continue;

        // Handle unsaved changes
        if (editor->document()->isModified()) {
            QMessageBox::StandardButton reply = QMessageBox::question(
                this,
                "Unsaved Changes",
                QString("Tab '%1' has unsaved changes. Close it?").arg(ui->editor_tab->tabText(i)),
                QMessageBox::Yes | QMessageBox::No
                );
            if (reply == QMessageBox::No)
                continue;
        }

        QString path = editor->currentFilePath();
        if (!path.isEmpty())
            openEditors.remove(path);

        ui->editor_tab->removeTab(i);
        tab->deleteLater();
    }
}



void MainWindow::on_actionLock_Tab_triggered()
{
    toggleLockTab();
}


void MainWindow::on_actionZoom_In_triggered()
{
    CodeEditor *editor = currentEditor(); // Your method to get active tab editor
    if(editor){
        editor->zoomIn(1);
    }
}


void MainWindow::on_actionZoom_Out_triggered()
{
    CodeEditor *editor = currentEditor(); // Your method to get active tab editor
    if(editor){
        editor->zoomOut(1);
    }
}


void MainWindow::on_actionReset_Zoom_triggered()
{
    CodeEditor *editor = currentEditor(); // Your method to get active tab editor
    if(editor){
        editor->resetZoom();
    }
}


void MainWindow::on_actionFull_Screen_triggered()
{
    if (isFullScreen()) {
        showNormal(); // exit full screen
        menuBar()->show();
        ui->toolBar->show();  // if you have a toolbar
        statusBar()->show();
    } else {
        showFullScreen(); // enter full screen
        menuBar()->hide();
        ui->toolBar->hide();
        statusBar()->hide();
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Escape) {
        if(isZenMode){
            on_actionZen_Mode_triggered();
        }
        if(isFullScreen())
        on_actionFull_Screen_triggered();
    } if (event->key() == Qt::Key_Alt && !menuBar()->isVisible()) {
        menuBar()->show();
    } else {
        QMainWindow::keyPressEvent(event);
    }
}



void MainWindow::on_actionZen_Mode_triggered()
{
    if (!isZenMode) {
        isZenMode = true;
        showFullScreen();

        menuBar()->hide();
        ui->toolBar->hide();
        ui->statusBar->hide();
        ui->widget_2->hide();
        ui->left_widget->hide();
    } else {
        isZenMode = false;
        showNormal();

        menuBar()->show();
        ui->toolBar->show();
        ui->statusBar->show();
        ui->widget_2->show();
        ui->left_widget->show();
    }
}

void MainWindow::on_actionToggle_Menubar_triggered()
{
    if(menuBar()->isVisible()){
        menuBar()->hide();
    }else{
        menuBar()->show();
    }
}


void MainWindow::on_actionPrimary_SideBar_triggered()
{
    if(ui->toolBar->isVisible()){
        ui->toolBar->hide();
    }else{
        ui->toolBar->show();
    }
}


void MainWindow::on_actionSecond_Sidebar_triggered()
{
    if(ui->symbol_table->isVisible()){
        ui->symbol_table->hide();
    }else{
        ui->symbol_table->show();
    }
}




void MainWindow::on_actionStatus_Bar_triggered()
{
    if(ui->statusBar->isVisible()){
        ui->statusBar->hide();
    }else{
        ui->statusBar->show();
    }
}


void MainWindow::on_actionWord_Wrap_toggled(bool arg1)
{
    if (CodeEditor *editor = currentEditor()) {  // Use your active editor getter
        if (arg1) {
            editor->setLineWrapMode(QPlainTextEdit::WidgetWidth); // Wrap at widget edge
        } else {
            editor->setLineWrapMode(QPlainTextEdit::NoWrap);
        }
    }
}

void MainWindow::setupCompilationTerminal() {
    if (!compilationTerminal) {
        compilationTerminal = new TerminalWidget(this);
        ui->terminal_tab->addTab(compilationTerminal, "Compilation");
    }
}


void MainWindow::on_actionCompilation_triggered()
{
    CodeEditor *editor = currentEditor();
    QString filePath;

    if (editor) {
        filePath = editor->currentFilePath();  // assumes your CodeEditor class has this
    }

    compilationTerminal->clear();
    if (filePath.isEmpty()) return;
    // Step 1: Create compilation terminal if not exists
    //if (!compilationTerminal) {
        // compilationTerminal = new TerminalWidget(this);
        int tabIndex = ui->terminal_tab->addTab(compilationTerminal,QIcon(":/compile.png"), "Compilation");
        ui->terminal_tab->setTabToolTip(tabIndex, "Compilation Output");
    //}

    // Step 2: Make terminal area visible if hidden
    if (!ui->terminal_widget->isVisible()) {
        ui->terminal_widget->setVisible(true);
    }

    // Step 3: Switch to compilation tab
    int index = ui->terminal_tab->indexOf(compilationTerminal);
    if (index != -1) {
        ui->terminal_tab->setCurrentIndex(index);
        compilationTerminal->clear();  // clear previous output
    }

    // Step 4: Ensure compilation manager exists
    if (!compilationManager) {
        compilationManager = new CompilationManager(this);
    } else {
        disconnect(compilationManager, nullptr, nullptr, nullptr);  // disconnect previous
    }

    // Step 5: Connect and compile
    connect(compilationManager, &CompilationManager::compilationOutput,
            this, [=](const QString &text) {
                compilationTerminal->appendText(text);  // your TerminalWidget method
            });

    compilationManager->compileFile(filePath);
}



void MainWindow::on_actionOpen_Terminal_triggered()
{
    // if(!ui->terminal_widget->isVisible()){
    //     ui->terminal_tab->destroyed(terminal1);
    //     // terminal1->clear();
    // QSettings settings("abc", "def");
    // QString lastDir= settings.value("lastOpenedDirectory").toString();
    // //TerminalWidget *terminal = new TerminalWidget(this, TerminalWidget::DefaultMode);
    // terminal1->setInitialDirectory(lastDir);
    // terminal1->startTerminal();
    // int tabIndex=ui->terminal_tab->addTab(terminal1,QIcon(":/terminal.png"),"terminal");
    // ui->terminal_tab->setCurrentIndex(tabIndex);
    // ui->terminal_widget->setVisible(true);
    // }
    // Remove previous terminal if any
    if (terminal1) {
        int index = ui->terminal_tab->indexOf(terminal1);
        if (index != -1)
            ui->terminal_tab->removeTab(index);

        terminal1->deleteLater();
        terminal1 = nullptr;
    }

    // Create fresh instance
    terminal1 = new PowerShellTerminalWidget(this);

    QSettings settings("abc", "def");
    QString lastDir = settings.value("lastOpenedDirectory").toString();
    terminal1->setInitialDirectory(lastDir);
    terminal1->startTerminal();

    int tabIndex = ui->terminal_tab->addTab(terminal1, QIcon(":/terminal.png"), "Terminal");
    ui->terminal_tab->setCurrentIndex(tabIndex);
    ui->terminal_widget->setVisible(true);
}


void MainWindow::on_actionRun_Without_Debugging_triggered()
{
    CodeEditor *editor = currentEditor();
    QString filePath;

    if (editor) {
        filePath = editor->currentFilePath();  // assumes your CodeEditor class has this
    }
    if (filePath.isEmpty()) return;
    QFileInfo fileInfo(filePath);
    QString fileName = fileInfo.fileName();
    ui->recentl_configured_combo->addItem(fileName);
    // Step 1: Create compilation terminal if not exists
    int tabIndex = ui->terminal_tab->addTab(runTerminalWidget,QIcon(":/play_red.png"), "Run");
    ui->terminal_tab->setTabToolTip(tabIndex, "Run Output");
    //}

    // Step 2: Make terminal area visible if hidden
    if (!ui->terminal_widget->isVisible()) {
        ui->terminal_widget->setVisible(true);
    }

    // Step 3: Switch to compilation tab
    int index = ui->terminal_tab->indexOf(runTerminalWidget);
    if (index != -1) {
        ui->terminal_tab->setCurrentIndex(index);// clear previous output
    }

    runTerminalWidget->compileBeforeRun(filePath);
}


void MainWindow::on_run_btn_clicked()
{

    on_actionRun_Without_Debugging_triggered();
}

void MainWindow::onEditorTabChanged(int index) {
    QWidget *widget = ui->editor_tab->widget(index);
    if (!widget)
        return;

    CodeEditor *current = qobject_cast<CodeEditor *>(widget);
    if (!current)
        return;

    editor = current; // update class-level editor pointer

    QString filePath = current->currentFilePath();
    if (filePath.isEmpty())
        return;

    // ✅ Highlight in file explorer
    if (model && ui->fileTreeView) {
        QModelIndex idx = model->index(filePath);
        if (idx.isValid()) {
            ui->fileTreeView->setCurrentIndex(idx);
            ui->fileTreeView->scrollTo(idx);
        }
    }

    // ✅ Refresh symbol table
    QString uri = QUrl::fromLocalFile(filePath).toString();
    lspClient->requestDocumentSymbols(uri);
}

