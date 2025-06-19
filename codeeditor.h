// #ifndef CODEEDITOR_H
// #define CODEEDITOR_H

// #include <QWidget>

// class CodeEditor
// {
// public:
//     CodeEditor();
// };

// #endif // CODEEDITOR_H


// lets builde your own custom code Editor
#pragma once

#include <QPlainTextEdit>
#include <QWidget>
#include <QSet>
#include<QList>
#include<QCompleter>
#include<QStringListModel>
class LineNumberArea;

class CodeEditor : public QPlainTextEdit {
    Q_OBJECT

public:
    explicit CodeEditor(QWidget *parent = nullptr);
    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();
    void toggleBreakpoint(int lineNumber);

    // client- server
    void addDiagnostic(int startLine, int startChar, int endLine, int endChar, const QString &severity,const QString &message);
    void clearDiagnostics();

    void showCompletionPopup(const QStringList &suggestions);

    void setCurrentFilePath(const QString &path);
    QString currentFilePath() const;
    QString getSymbolUnderCursor();
    void renameSymbolInDocument(const QString &oldName, const QString &newName);
    int changeAllOccurrences(const QString &target, const QString &replacement);
    QTextCursor textCursorForRange(int startLine, int startChar, int endLine, int endChar);
    void zoomIn(int range);
    void zoomOut(int range);
    void resetZoom();

// signals:
//     void requestCompletion();  // ✅ Must be under "signals:"


protected:
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *e) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &, int);
    void insertCompletion(const QString &completion);


signals:
    void requestCompletion(const QString &uri, int line, int character);

private:
    QWidget *lineNumberArea;
    QSet<int> breakpoints;

    QString textUnderCursor() const;

    QCompleter *c = nullptr;

    // client - server
    QList<QTextEdit::ExtraSelection> diagnosticSelections;


    QCompleter *completer = nullptr;
    QStringListModel *completionModel = nullptr;

    struct DiagnosticEntry {
        int startLine;
        int startChar;
        int endLine;
        int endChar;
        QString message;
    };

    QVector<DiagnosticEntry> diagnostics;
    QString filePath;

};


