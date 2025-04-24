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

class LineNumberArea;

class CodeEditor : public QPlainTextEdit {
    Q_OBJECT

public:
    CodeEditor(QWidget *parent = nullptr);
    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();
    void toggleBreakpoint(int lineNumber);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &, int);

private:
    QWidget *lineNumberArea;
    QSet<int> breakpoints;
};
