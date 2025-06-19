#include "codeeditor.h"
#include <QPainter>
#include<QListView>
#include <QTextBlock>
#include<QKeyEvent>
#include <QMouseEvent>
#include<QAbstractItemView>
#include<QScrollBar>
#include<QString>
#include<QStringListModel>
#include<QFocusEvent>
#include<QFont>
#include<QToolTip>
#include<mainwindow.h>
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

CodeEditor::CodeEditor(QWidget *parent) : QPlainTextEdit(parent)   {

    setMouseTracking(true);
    completionModel = new QStringListModel(this);
    completer = new QCompleter(completionModel, this);
    completer->setWidget(this);  // this = CodeEditor
    completer->setCompletionMode(QCompleter::PopupCompletion);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
   // QListView *popup = completer->popup();
    completer->popup()->setStyleSheet(R"(
    QListView {
        background-color: #1e1e1e;
        color: white;
        font-family: Consolas;
        font-size: 14px;
        border: 1px solid #555;
        padding: 2px;
    }
    QListView::item {
        padding: 4px 8px;
    }
    QListView::item:selected {
        background-color: #2d2d30;
        color: #dcdcdc;
    }
)");






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

    setLineWrapMode(QPlainTextEdit::NoWrap);  // Default word wrap enabled
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
            border: none;
        }
    QScrollBar:vertical {
        background: transparent;
        width: 5px;
        margin: 0px 0px 0px 0px;
    }

    QScrollBar::handle:vertical {
        background-color: #3399FF;
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

    connect(completer, QOverload<const QString &>::of(&QCompleter::activated),
            this, &CodeEditor::insertCompletion);


}

int CodeEditor::lineNumberAreaWidth() {
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int space = 5 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits+24;  // + space for breakpoint

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
        selection.format.setBackground(QColor(42,46,43));
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }
    // setExtraSelections(extraSelections);
    extraSelections += diagnosticSelections;
    setExtraSelections(extraSelections);
}
/*
void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event) {
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), QColor(30, 30, 30));


    QFont lineFont = this->font();  // get current font (zoomed)
    painter.setFont(lineFont);
    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = static_cast<int>(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + static_cast<int>(blockBoundingRect(block).height());
    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(QColor(120, 120, 120)); // line number color
            QFont font;
            font.setBold(true);
            painter.setFont(font);
            painter.drawText(0, top, lineNumberArea->width()-6, fontMetrics().height(),
                             Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + static_cast<int>(blockBoundingRect(block).height());
        ++blockNumber;
    }
}*/


void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event) {
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), QColor("#1e1e1e")); // background

    // Set font to match editor's font
    QFont lineFont = this->font();  // get current font (zoomed)
    painter.setFont(lineFont);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = static_cast<int>(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + static_cast<int>(blockBoundingRect(block).height());

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(QColor("#888888")); // line number color
            painter.drawText(0, top, lineNumberArea->width() - 5, fontMetrics().height(),
                             Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + static_cast<int>(blockBoundingRect(block).height());
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

void CodeEditor::keyPressEvent(QKeyEvent *e) {
    // ✅ Let completer handle navigation and selection keys
    if (completer && completer->popup()->isVisible()) {
        switch (e->key()) {
        case Qt::Key_Enter:
        case Qt::Key_Return:
        case Qt::Key_Escape:
        case Qt::Key_Tab:
        case Qt::Key_Backtab:
        case Qt::Key_Up:
        case Qt::Key_Down:
            e->ignore();  // Let completer handle it
            return;
        default:
            break;
        }
    }


    // new add
    const QChar keyChar = e->text().isEmpty() ? QChar() : e->text().at(0);

    if (keyChar == '(') {
        insertPlainText("()");
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::Left);
        setTextCursor(cursor);
        return;  // prevent default handling
    }

    // You can expand this for other brackets as well
    if (keyChar == '{') {
        insertPlainText("{}");
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::Left);
        setTextCursor(cursor);
        return;
    }

    if (keyChar == '[') {
        insertPlainText("[]");
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::Left);
        setTextCursor(cursor);
        return;
    }

    if (keyChar == '"') {
        insertPlainText("\"\"");
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::Left);
        setTextCursor(cursor);
        return;
    }

    if (keyChar == '\'') {
        insertPlainText("''");
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::Left);
        setTextCursor(cursor);
        return;
    }

    // ✅ Let the base class update the cursor & buffer
    QPlainTextEdit::keyPressEvent(e);

    // ✅ Hide completer if prefix was cleared or deleted
    if (completer && completer->popup()->isVisible()) {
        QTextCursor cursor = textCursor();
        cursor.select(QTextCursor::WordUnderCursor);
        QString currentWord = cursor.selectedText();

        QString prefix = completer->completionPrefix();

        // Hide if nothing left to complete
        if (currentWord.isEmpty() || prefix.isEmpty() ||
            e->key() == Qt::Key_Backspace || e->key() == Qt::Key_Delete) {
            completer->popup()->hide();
            // return;
        }
    }

    // ✅ Only trigger LSP completion for meaningful input
    if (!(e->text().size() == 1 && (e->text().at(0).isLetterOrNumber() || e->text() == "."))) {
        return;
    }

    // ✅ Send LSP completion request
    int line = textCursor().blockNumber();
    int character = textCursor().positionInBlock();
    QString uri = QUrl::fromLocalFile(currentFilePath()).toString();
    emit requestCompletion(uri, line, character);
}

void CodeEditor::addDiagnostic(int startLine, int startChar, int endLine, int endChar, const QString &severity, const QString &message) {
    DiagnosticEntry entry = { startLine, startChar, endLine, endChar, message };
    diagnostics.append(entry);

    QTextBlock block = document()->findBlockByNumber(startLine);
    if (!block.isValid()) return;

    QTextCursor cursor(block);

    // 🧠 Clamp character positions within block bounds
    int blockLength = block.length();
    int startPos = block.position() + qMin(startChar, blockLength - 1);
    int endPos = block.position() + qMin(endChar, blockLength - 1);

    // 🛠 Expand zero-length ranges
    if (startLine == endLine && startChar == endChar) {
        endPos = startPos + 1;
    }

    // ❗ Fallback: highlight entire line if invalid range
    if (startPos >= endPos || endPos > block.position() + block.length()) {
        cursor.setPosition(block.position());
        cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    } else {
        cursor.setPosition(startPos);
        cursor.setPosition(endPos, QTextCursor::KeepAnchor);
    }

    // 🎨 Customize format based on severity
    QTextCharFormat format;
    format.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);

    if (severity == "Error") {
        format.setUnderlineColor(QColor(255,0,0));  // 🔴 red
    } else if (severity == "Warning") {
        format.setUnderlineColor(QColor("#ffaa00"));  // 🟠 orange/yellow
    } else {
        format.setUnderlineColor(Qt::gray);  // fallback
    }

    // Add to selection
    QTextEdit::ExtraSelection selection;
    selection.cursor = cursor;
    selection.format = format;

    diagnosticSelections.append(selection);

    // ✅ Combine with other selections (like current line, search, etc.)
    setExtraSelections(diagnosticSelections + this->extraSelections());
}


/*
    void CodeEditor::addDiagnostic(int startLine, int startChar, int endLine, int endChar, const QString &severity,const QString &message) {
        DiagnosticEntry entry = { startLine, startChar, endLine, endChar, message };
        diagnostics.append(entry);

        QTextBlock block = document()->findBlockByNumber(startLine);
        if (!block.isValid()) return;

        QTextCursor cursor(block);

        // 🧠 Clamp character positions within block bounds
        int blockLength = block.length();
        int startPos = block.position() + qMin(startChar, blockLength - 1);
        int endPos = block.position() + qMin(endChar, blockLength - 1);

        // 🛠 Expand zero-length ranges
        if (startLine == endLine && startChar == endChar) {
            endPos = startPos + 1;
        }

        // ❗ Fallback: if range is still invalid, highlight entire line
        if (startPos >= endPos || endPos > block.position() + block.length()) {
            cursor.setPosition(block.position());
            cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        } else {
            cursor.setPosition(startPos);
            cursor.setPosition(endPos, QTextCursor::KeepAnchor);
        }

        QTextEdit::ExtraSelection selection;
        selection.cursor = cursor;
        selection.format.setUnderlineColor(Qt::red);
        selection.format.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);

        diagnosticSelections.append(selection);

        // ✅ Combine with other selections (like current line)
        setExtraSelections(diagnosticSelections + this->extraSelections());
    }
*/

    void CodeEditor::clearDiagnostics()
    {
        //qDebug()<< "Clear Diagonostic called...";
        diagnostics.clear();
        diagnosticSelections.clear();
        // setExtraSelections(diagnosticSelections);
        highlightCurrentLine();
    }


    void CodeEditor::showCompletionPopup(const QStringList &suggestions) {
        if (!completer || !completionModel)
            return;

        if (suggestions.isEmpty())
            return;

        // // Correctly get the word under the cursor
        // QTextCursor cursor = textCursor();
        // cursor.select(QTextCursor::WordUnderCursor);
        // QString currentWord = cursor.selectedText();
        // completer->setCompletionPrefix(currentWord);

        completionModel->setStringList(suggestions);

        // Show popup at cursor
        QRect cr = cursorRect();
        cr.setWidth(completer->popup()->sizeHintForColumn(0) +
                    completer->popup()->verticalScrollBar()->sizeHint().width());

        completer->complete(cr);  // Show popup
        // qDebug() << "📌 Completion popup triggered at cursor. Prefix:" << currentWord;
    }





    void CodeEditor::setCurrentFilePath(const QString &path) {
        filePath = path;
    }

    QString CodeEditor::currentFilePath() const {
        return filePath;
    }

    void CodeEditor::insertCompletion(const QString &completion) {
        if (!completer)
            return;

        QTextCursor cursor = textCursor();

        // Remove the typed prefix (like "su")
        //int prefixLength = completer->completionPrefix().length();
        // cursor.movePosition(QTextCursor::StartOfWord, QTextCursor::KeepAnchor, 1);
        cursor.movePosition(QTextCursor::PreviousWord, QTextCursor::KeepAnchor);

        cursor.insertText(completion);  // Replace "su" with "sum"
        setTextCursor(cursor);
    }


    void CodeEditor::mouseMoveEvent(QMouseEvent *event) {
        QPlainTextEdit::mouseMoveEvent(event);

        QTextCursor cursor = cursorForPosition(event->pos());
        int line = cursor.blockNumber();
        int column = cursor.positionInBlock();

        for (const DiagnosticEntry &entry : diagnostics) {
            if (line >= entry.startLine && line <= entry.endLine) {
                if ((line != entry.endLine || column <= entry.endChar) &&
                    (line != entry.startLine || column >= entry.startChar)) {
                    QToolTip::showText(event->globalPosition().toPoint(), entry.message, this);
                    return;
                }
            }
        }

        QToolTip::hideText();  // no message to show
    }

    QString CodeEditor::getSymbolUnderCursor() {
        QTextCursor cursor = this->textCursor();
        cursor.select(QTextCursor::WordUnderCursor);
        return cursor.selectedText();
    }
    void CodeEditor::renameSymbolInDocument(const QString &oldName, const QString &newName) {
        QTextDocument *doc = document();
        QTextCursor cursor(doc);
        QRegularExpression wordRegex(QString("\\b%1\\b").arg(QRegularExpression::escape(oldName)));

        QVector<QTextCursor> matches;

        // 1. Collect all match positions first
        while (!cursor.isNull() && !cursor.atEnd()) {
            cursor = doc->find(wordRegex, cursor);
            if (!cursor.isNull()) {
                matches.append(cursor);
            }
        }

        // 2. Replace in reverse order (from bottom to top)
        cursor.beginEditBlock();  // Enables single-step undo
        for (int i = matches.size() - 1; i >= 0; --i) {
            QTextCursor c = matches[i];
            c.insertText(newName);
        }
        cursor.endEditBlock();
    }

    int CodeEditor::changeAllOccurrences(const QString &target, const QString &replacement) {
        QTextDocument *doc = document();
        QTextCursor cursor(doc);
        QRegularExpression wordRegex(QString("\\b%1\\b").arg(QRegularExpression::escape(target)));

        QVector<QTextCursor> matches;

        while (!cursor.isNull() && !cursor.atEnd()) {
            cursor = doc->find(wordRegex, cursor);
            if (!cursor.isNull()) {
                matches.append(cursor);
            }
        }

        cursor.beginEditBlock();
        for (int i = matches.size() - 1; i >= 0; --i) {
            QTextCursor c = matches[i];
            c.insertText(replacement);
        }
        cursor.endEditBlock();

        return matches.size();
    }

    QTextCursor CodeEditor::textCursorForRange(int startLine, int startChar, int endLine, int endChar) {
        QTextBlock startBlock = document()->findBlockByNumber(startLine);
        QTextBlock endBlock = document()->findBlockByNumber(endLine);

        QTextCursor cursor(document());
        cursor.setPosition(startBlock.position() + startChar);
        cursor.setPosition(endBlock.position() + endChar, QTextCursor::KeepAnchor);
        return cursor;
    }

    void CodeEditor::zoomIn(int range) {
        QFont font = this->font();
        int size = font.pointSize();
        font.setPointSize(size + range);
        if(size+range>18){
            return;
        }
        this->setFont(font);
        this->document()->setDefaultFont(font);

        updateLineNumberAreaWidth(0);
        lineNumberArea->update();
    }

    void CodeEditor::zoomOut(int range) {
        QFont font = this->font();
        int size = font.pointSize();
        if (size - range > 11)
            font.setPointSize(size - range);
        this->setFont(font);
        this->document()->setDefaultFont(font);

        updateLineNumberAreaWidth(0);
        lineNumberArea->update();
    }

    void CodeEditor::resetZoom() {
        QFont defaultFont("JetBrains Mono");
        defaultFont.setStyleHint(QFont::Monospace);
        defaultFont.setFixedPitch(true);
        defaultFont.setPointSize(11); // default size
        defaultFont.setStyleStrategy(QFont::PreferAntialias);

        this->setFont(defaultFont);
        this->document()->setDefaultFont(defaultFont);

        const int tabStop = 4; // spaces
        QFontMetrics metrics(defaultFont);
        setTabStopDistance(tabStop * metrics.horizontalAdvance(' '));

        updateLineNumberAreaWidth(0);
        lineNumberArea->update();
    }

