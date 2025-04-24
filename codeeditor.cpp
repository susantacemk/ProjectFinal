#include "codeeditor.h"
#include <QPainter>
#include <QTextBlock>
#include <QMouseEvent>
#include<QScrollBar>
class LineNumberArea : public QWidget {
public:
    LineNumberArea(CodeEditor *editor) : QWidget(editor), codeEditor(editor) {}
    QSize sizeHint() const override { return QSize(codeEditor->lineNumberAreaWidth(), 0); }

protected:
    void paintEvent(QPaintEvent *event) override {
        codeEditor->lineNumberAreaPaintEvent(event);
    }

private:
    CodeEditor *codeEditor;
};

CodeEditor::CodeEditor(QWidget *parent) : QPlainTextEdit(parent) {

    lineNumberArea = new LineNumberArea(this);

    connect(this, &CodeEditor::blockCountChanged, this, &CodeEditor::updateLineNumberAreaWidth);
    connect(this, &CodeEditor::updateRequest, this, &CodeEditor::updateLineNumberArea);
    connect(this, &CodeEditor::cursorPositionChanged, this, &CodeEditor::highlightCurrentLine);

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();

    QFont font("JetBrains Mono"); // or JetBrains Mono, Consolas, etc.
    font.setStyleHint(QFont::Monospace);
    font.setFixedPitch(true);
    font.setPointSize(11);
    font.setStyleStrategy(QFont::PreferAntialias);
    setFont(font);

    // Optional: set tab width to match font
    const int tabStop = 4; // spaces
    QFontMetrics metrics(font);
    setTabStopDistance(tabStop * metrics.horizontalAdvance(' '));

    // Set smooth scrolling
    verticalScrollBar()->setSingleStep(1);
    horizontalScrollBar()->setSingleStep(1);
    setStyleSheet(R"(
    QPlainTextEdit {
            background-color: #1e1e1e;
            color: #d4d4d4;
            font: 11pt "JetBrains Mono";
            border: none;
        }
    QScrollBar:vertical {
        background: transparent;
        width: 5px;
        margin: 0px 0px 0px 0px;
    }

    QScrollBar::handle:vertical {
        background-color: #3399FF; /* soft blue */
        min-height: 20px;
        border-radius: 2px;
    }

    QScrollBar::handle:vertical:hover {
        background-color: #66b2ff;
    }

    QScrollBar:horizontal {
        background: transparent;
        height: 5px;
        margin: 0px 0px 0px 0px;
    }

    QScrollBar::handle:horizontal {
        background-color: #3399FF;
        min-width: 20px;
        border-radius: 2px;
    }

    QScrollBar::handle:horizontal:hover {
        background-color: #66b2ff;
    }
)");

}

int CodeEditor::lineNumberAreaWidth() {
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int space = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits+14;  // + space for breakpoint

    return space;
}

void CodeEditor::updateLineNumberAreaWidth(int) {
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy) {
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void CodeEditor::resizeEvent(QResizeEvent *event) {
    QPlainTextEdit::resizeEvent(event);
    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void CodeEditor::highlightCurrentLine() {
    QList<QTextEdit::ExtraSelection> extraSelections;
    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;
        selection.format.setBackground(QColor(42,42,42));
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }
    setExtraSelections(extraSelections);
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event) {
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), QColor(30, 30, 30));

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = static_cast<int>(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + static_cast<int>(blockBoundingRect(block).height());
    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(Qt::white);
            painter.drawText(0, top, lineNumberArea->width(), fontMetrics().height(),
                             Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

void CodeEditor::mousePressEvent(QMouseEvent *event) {
    if (event->position().x() < lineNumberAreaWidth()) {
       int y = static_cast<int>(event->position().y());
        QTextBlock block = firstVisibleBlock();
        int top = static_cast<int>(blockBoundingGeometry(block).translated(contentOffset()).top());
        int bottom = top + static_cast<int>(blockBoundingRect(block).height());
        while (block.isValid() && top <= y) {
            if (block.isVisible() && y <= bottom) {
                toggleBreakpoint(block.blockNumber() + 1);
                break;
            }
            block = block.next();
            top = bottom;
            bottom = top + static_cast<int>(blockBoundingRect(block).height());
        }
    }
    QPlainTextEdit::mousePressEvent(event);
}

void CodeEditor::toggleBreakpoint(int lineNumber) {
    if (breakpoints.contains(lineNumber))
        breakpoints.remove(lineNumber);
    else
        breakpoints.insert(lineNumber);
    lineNumberArea->update();
}

