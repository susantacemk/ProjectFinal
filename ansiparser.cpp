#include "ansiparser.h"
#include <QTextCursor>
#include <QDebug>

AnsiParser::AnsiParser(QObject *parent) : QObject(parent)
{
    // Initialize default state
    m_state.format = QTextCharFormat();
    m_state.foreground = Qt::black;
    m_state.background = Qt::white;
}

void AnsiParser::parse(const QByteArray &data, QTextCursor &cursor)
{
    for (char ch : data) {
        switch (m_mode) {
        case Normal:
            if (ch == 0x1B) {  // ESC character
                m_mode = Escape;
            } else {
                // Insert normal character with current format
                QTextCharFormat oldFormat = cursor.charFormat();
                cursor.setCharFormat(m_state.format);
                cursor.insertText(QString(ch));
                cursor.setCharFormat(oldFormat);
            }
            break;

        case Escape:
            if (ch == '[') {
                m_mode = CSI;
                m_csiParams.clear();
                m_csiIntermediate.clear();
            } else if (ch == ']') {
                m_mode = OSC;
                m_oscBuffer.clear();
            } else {
                handleEscapeSequence(ch);
                m_mode = Normal;
            }
            break;

        case CSI:
            handleCsiSequence(ch,cursor);
            break;

        case OSC:
            handleOscSequence(ch);
            break;
        }
    }
}

void AnsiParser::handleEscapeSequence(char ch)
{
    switch (ch) {
    case 'm':  // Reset
        m_state.format = QTextCharFormat();
        m_state.bold = false;
        m_state.underline = false;
        m_state.inverse = false;
        break;
        // Add other escape sequences as needed
    }
}

void AnsiParser::handleCsiSequence(char ch,QTextCursor &cursor)
{
    if (ch >= '0' && ch <= '9') {
        if (m_csiIntermediate.isEmpty() && !m_csiParams.isEmpty()) {
            m_csiParams.last() = m_csiParams.last() * 10 + (ch - '0');
        } else {
            m_csiIntermediate += ch;
        }
    } else if (ch == ';') {
        m_csiParams.append(0);
    } else {
        // Final character - process command
        switch (ch) {
        case 'm':  // SGR - Select Graphic Rendition
            applyGraphicRendition();
            break;
        case 'H':  // CUP - Cursor Position
        case 'f':
            setCursorPosition(cursor);
            break;
        case 'A':  // CUU - Cursor Up
            processCursorMovement(cursor, -1, 1);  // Move up by n (default 1)
            break;
        case 'B':  // CUD - Cursor Down
            processCursorMovement(cursor, 1, 1);    // Move down by n (default 1)
            break;
        case 'C':  // CUF - Cursor Forward
           // processCursorMovement(cursor, 1, 1);    // Move right by n (default 1)
            break;
        case 'D':  // CUB - Cursor Back
            processCursorMovement(cursor, -1, 1);    // Move left by n (default 1)
            break;
        case 'J':  // ED - Erase Display
            processErase(cursor, m_csiParams.isEmpty() ? 0 : m_csiParams.first());
            break;
        case 'K':  // EL - Erase Line
            processErase(cursor, m_csiParams.isEmpty() ? 0 : m_csiParams.first());
            break;
        }
        m_mode = Normal;
    }
}

void AnsiParser::handleOscSequence(char ch)
{
    if (ch == '\a' || ch == '\x1B') {  // BEL or ESC
        // OSC sequence finished
        if (m_oscBuffer.startsWith("0;") || m_oscBuffer.startsWith("2;")) {
            emit titleChanged(m_oscBuffer.mid(2));
        }
        m_mode = Normal;
    } else {
        m_oscBuffer += ch;
    }
}

void AnsiParser::applyGraphicRendition()
{
    if (m_csiParams.isEmpty()) {
        m_csiParams.append(0);  // Default to 0 if no parameters
    }

    for (int param : m_csiParams) {
        switch (param) {
        case 0:  // Reset
            m_state.format = QTextCharFormat();
            m_state.bold = false;
            m_state.underline = false;
            m_state.inverse = false;
            break;
        case 1:  // Bold
            m_state.bold = true;
            m_state.format.setFontWeight(QFont::Bold);
            break;
        case 4:  // Underline
            m_state.underline = true;
            m_state.format.setFontUnderline(true);
            break;
        case 7:  // Inverse
            m_state.inverse = true;
            break;
        case 22:  // Normal intensity
            m_state.bold = false;
            m_state.format.setFontWeight(QFont::Normal);
            break;
        case 24:  // Underline off
            m_state.underline = false;
            m_state.format.setFontUnderline(false);
            break;
        case 27:  // Inverse off
            m_state.inverse = false;
            break;
        case 30: case 31: case 32: case 33: case 34: case 35: case 36: case 37:
            // Foreground colors
            m_state.foreground = ansiColor(param - 30);
            m_state.format.setForeground(m_state.foreground);
            break;
        case 39:  // Default foreground
            m_state.foreground = Qt::black;
            m_state.format.setForeground(m_state.foreground);
            break;
        case 40: case 41: case 42: case 43: case 44: case 45: case 46: case 47:
            // Background colors
            m_state.background = ansiColor(param - 40);
            m_state.format.setBackground(m_state.background);
            break;
        case 49:  // Default background
            m_state.background = Qt::white;
            m_state.format.setBackground(m_state.background);
            break;
        }
    }
}

QColor AnsiParser::ansiColor(int index)
{
    static const QColor colors[] = {
        Qt::black,
        Qt::red,
        Qt::green,
        Qt::yellow,
        Qt::blue,
        Qt::magenta,
        Qt::cyan,
        Qt::white
    };
    return (index >= 0 && index < 8) ? colors[index] : Qt::black;
}

void AnsiParser::setCursorPosition(QTextCursor &cursor)
{
    int row = (m_csiParams.size() > 0) ? qMax(1, m_csiParams[0]) : 1;
    int col = (m_csiParams.size() > 1) ? qMax(1, m_csiParams[1]) : 1;

    // Move cursor to specified position
    cursor.movePosition(QTextCursor::Start);
    for (int i = 1; i < row; i++) {
        cursor.movePosition(QTextCursor::Down);
    }
    cursor.movePosition(QTextCursor::StartOfLine);
    for (int i = 1; i < col; i++) {
        cursor.movePosition(QTextCursor::Right);
    }
}

void AnsiParser::processCursorMovement(QTextCursor &cursor, int param, int defaultVal)
{
    int n = (param > 0) ? param : defaultVal;
    if (n <= 0) return;

    if (param > 0) {
        // Move down/right
        for (int i = 0; i < n; i++) {
            cursor.movePosition(param == defaultVal ? QTextCursor::Right : QTextCursor::Down);
        }
    } else {
        // Move up/left
        for (int i = 0; i < n; i++) {
            cursor.movePosition(param == defaultVal ? QTextCursor::Left : QTextCursor::Up);
        }
    }
}

void AnsiParser::processErase(QTextCursor &cursor, int param)
{
    switch (param) {
    case 0:  // Erase from cursor to end of screen
        cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
        cursor.removeSelectedText();
        break;
    case 1:  // Erase from start to cursor
        cursor.movePosition(QTextCursor::Start, QTextCursor::KeepAnchor);
        cursor.removeSelectedText();
        break;
    case 2:  // Erase entire screen
        cursor.select(QTextCursor::Document);
        cursor.removeSelectedText();
        break;
    }
}
