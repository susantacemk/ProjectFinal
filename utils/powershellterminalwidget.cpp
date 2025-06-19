// PowerShellTerminalWidget.cpp
#include "powershellterminalwidget.h"
#include <QKeyEvent>
#include <QDir>
#include <QRegularExpression>
#include <QDebug>
#include<QApplication>
PowerShellTerminalWidget::PowerShellTerminalWidget(QWidget *parent)
    : QPlainTextEdit(parent), m_process(new QProcess(this))
{
    setReadOnly(false);
    setUndoRedoEnabled(false);
    setWordWrapMode(QTextOption::NoWrap);
    setFont(QFont("Consolas", 11));

    m_defaultFormat.setForeground(Qt::white);
    m_errorFormat.setForeground(Qt::red);

    connect(m_process, &QProcess::readyRead, this, &PowerShellTerminalWidget::handleOutput);
    connect(m_process, &QProcess::readyReadStandardError, this, &PowerShellTerminalWidget::handleErrorOutput);


    m_promptFormat.setForeground(QColor(0,255,255));  // cyan
    m_outputFormat.setForeground(QColor(255,255,255));  // white (or any)

    m_historyIndex = -1;
}

PowerShellTerminalWidget::~PowerShellTerminalWidget() {
    if (m_process) {
        if (m_process->state() == QProcess::Running) {
            m_process->terminate();
            if (!m_process->waitForFinished(2000)) {
                m_process->kill();
                m_process->waitForFinished();
            }
        }
        delete m_process;
        m_process = nullptr;
    }
}

void PowerShellTerminalWidget::setInitialDirectory(const QString &path) {
    if (QDir(path).exists()) {
        m_currentDir = QDir::toNativeSeparators(path);
    } else {
        m_currentDir = QDir::currentPath();
    }
}

void PowerShellTerminalWidget::startTerminal() {
    if (m_process->state() != QProcess::NotRunning)
        return;

    QString escapedPath = m_currentDir;
    escapedPath.replace("\"", "\"\"");

    // QString initScript = QString(
    //                          "function prompt { \"$PWD> \" }; cd \"%1\"; Write-Host \"$PWD> \""
    //                          ).arg(escapedPath);
    QString initScript = QString(
                             "function prompt {  \"$PWD> \"}; cd \"%1\""
                             ).arg(escapedPath);

    m_process->setWorkingDirectory(m_currentDir);
    m_process->start("powershell.exe", QStringList()
                                           << "-NoLogo"
                                           << "-NoExit"
                                           << "-Command" << initScript);
}

void PowerShellTerminalWidget::insertPrompt() {
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::End);
    QString prompt = m_currentDir + ">";
    cursor.insertText(prompt,m_promptFormat);
    setTextCursor(cursor);
    m_promptPosition = textCursor().position();
    ensureCursorVisible();
}

void PowerShellTerminalWidget::handleOutput() {
    QString output = QString::fromLocal8Bit(m_process->readAllStandardOutput());
    QStringList lines = output.split(QRegularExpression("[\r\n]"), Qt::KeepEmptyParts);

    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::End);

    for (const QString &line : lines) {
        QString trimmed = line.trimmed();
        // format specifier
        QTextCharFormat format = (trimmed == m_currentDir + ">")
                                     ? m_promptFormat     // our custom prompt style
                                     : m_outputFormat;
        if (trimmed == m_currentDir + ">") {
            m_waitingForInput = false;
            m_promptPosition = cursor.position() + line.length() - 1; // after inserting this line
        }
        if (trimmed.isEmpty() || trimmed == m_lastCommand)
            continue;

        cursor.insertText(line + "\n", format);
    }
    ensureCursorVisible();
}

void PowerShellTerminalWidget::handleErrorOutput() {
    QString errorOutput = QString::fromLocal8Bit(m_process->readAllStandardError());
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::End);

    QTextCharFormat errorFormat;
    errorFormat.setForeground(Qt::red);

    cursor.insertText(errorOutput, errorFormat);
    ensureCursorVisible();
}

void PowerShellTerminalWidget::executeCommand(const QString &command) {
    if (m_process->state() != QProcess::Running || command.trimmed().isEmpty()) return;
    m_lastCommand = command.trimmed();

    QString normalizedCmd = m_lastCommand.trimmed();
    // Handle unquoted cd path with spaces
    if (normalizedCmd.startsWith("cd ")) {
        QString path = normalizedCmd.mid(3).trimmed();
        if (path.contains(' ') && !path.startsWith("\"")) {
            normalizedCmd = "cd \"" + path + "\"";
        }
    }

    if (!m_lastCommand.isEmpty()) {
        m_commandHistory.append(m_lastCommand);
        m_historyIndex = m_commandHistory.size();
    }

    if (m_lastCommand == "clear" || m_lastCommand=="cls") {
        clear();
        insertPrompt();
        return;
    }

    if (m_lastCommand == "exit") {
        m_process->terminate();
        m_process->waitForFinished(2000);
        m_process->close();
        appendPlainText("\n[Terminal closed]\n");
        setReadOnly(true);
        return;
    }


    if (m_lastCommand.endsWith(".exe") || m_lastCommand.endsWith(".py")) {
        m_waitingForInput = true;
    }

    m_process->write((m_lastCommand + "\r\n").toLocal8Bit());
}

void PowerShellTerminalWidget::keyPressEvent(QKeyEvent *event) {
    QTextCursor cursor = textCursor();
    if (m_waitingForInput) {
        // Handle input char by char
        if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
            m_process->write("\n");
            // insertPlainText("\n");
            cursor.movePosition(QTextCursor::End);
            return;
        } else {
            QString inputChar = event->text();
            m_process->write(inputChar.toLocal8Bit());
            insertPlainText(inputChar);
            cursor.movePosition(QTextCursor::End);
            return;
        }
    }
    if (event->key() == Qt::Key_Backspace) {
        if (textCursor().position() <= m_promptPosition) {
            QApplication::beep();
            return; // Block backspace if at or before prompt
        } else {
            cursor.deletePreviousChar();
            setTextCursor(cursor);
            return;
        }
    }
/*
    if (event->key() == Qt::Key_Up && !m_commandHistory.isEmpty()) {
        if (m_historyIndex > 0) m_historyIndex--;
        replaceCurrentCommand(m_commandHistory[m_historyIndex]);
        return;
    }

    if (event->key() == Qt::Key_Down && !m_commandHistory.isEmpty()) {
        if (m_historyIndex < m_commandHistory.size() - 1)
            m_historyIndex++;
        replaceCurrentCommand(m_commandHistory[m_historyIndex]);
        return;
    }
*/
    // if (event->key() == Qt::Key_Tab) {
    //     handleTabCompletion();
    //     return;
    // }

    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
        QString line = cursor.selectedText();
        QString command = line.section('>', 1).trimmed();
        executeCommand(command);
        return;
    }


    QPlainTextEdit::keyPressEvent(event);
}


bool PowerShellTerminalWidget::isPromptEcho(const QString &line) const {
    QString prompt = m_currentDir + ">";
    QString cleaned = line.trimmed();

    return cleaned == prompt ||
           cleaned.startsWith("PS ") ||
           cleaned.contains("function prompt") ||
           cleaned.contains("cd ");
}

void PowerShellTerminalWidget::replaceCurrentCommand(const QString &command) {
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::EndOfLine);
    cursor.select(QTextCursor::BlockUnderCursor);
    QString fullLine = cursor.selectedText();
    int promptIndex = fullLine.indexOf('>');
    if (promptIndex != -1) {
        cursor.movePosition(QTextCursor::End);
        cursor.setPosition(m_promptPosition, QTextCursor::KeepAnchor);
        // cursor.removeSelectedText();
        cursor.insertText(command);
    }
    setTextCursor(cursor);
}


void PowerShellTerminalWidget::handleTabCompletion() {
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
    QString line = cursor.selectedText().trimmed();
    QString input = line.section('>', 1).trimmed();  // strip prompt
    QStringList tokens = input.split(QRegularExpression("\\s+"));

    if (tokens.isEmpty())
        return;

    QString current = tokens.last();
    QStringList matches;

    if (tokens.size() == 1) {
        // First word: match from available commands
        for (const QString &cmd : m_availableCommands) {
            if (cmd.startsWith(current)) {
                matches << cmd;
            }
        }
    } else {
        // Subsequent: file completion
        QDir dir(m_currentDir);
        QStringList files = dir.entryList(QDir::Files | QDir::Executable, QDir::Name);
        for (const QString &file : files) {
            if (file.startsWith(current)) {
                matches << file;
            }
        }
    }

    if (matches.size() == 1) {
        // Replace the current word with the match
        QString replacement = matches.first();
        int lastWordStart = line.lastIndexOf(current);
        cursor.movePosition(QTextCursor::End);
        cursor.setPosition(m_promptPosition + lastWordStart, QTextCursor::KeepAnchor);
        cursor.removeSelectedText();
        cursor.insertText(replacement);
        setTextCursor(cursor);
    } else if (matches.size() > 1) {
        // Show options below, reinsert prompt
        appendPlainText("\n" + matches.join("  "));
        insertPrompt();
    }
}
