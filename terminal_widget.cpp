
#include "terminal_widget.h"
#include <QScrollBar>
#include <QDir>
#include <QDebug>
#include <QMenu>
#include <QAction>
#include <QClipboard>
#include <QApplication>
#include <QTextCursor>
#include <QPainter>
#include <QSettings>
#include <QStandardPaths>
#include <QProcessEnvironment>
#include<QRect>

#ifdef Q_OS_UNIX
#include <unistd.h>
#include <sys/ioctl.h>
#include <termios.h>
#endif

TerminalWidget::TerminalWidget(QWidget *parent, TerminalMode mode)
    : QPlainTextEdit(parent), m_mode(mode), m_ansiParser(this)
{
    // Basic widget setup
    setObjectName("TerminalWidget");
    setReadOnly(false);
    setUndoRedoEnabled(false);
    setWordWrapMode(QTextOption::NoWrap);
    setFrameShape(QFrame::NoFrame);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setFocusPolicy(Qt::StrongFocus);
    setWordWrapMode(QTextOption::NoWrap);
    setFont(QFont("Courier New", 11));
    setCursorWidth(2);
    // Initialize formats
    m_defaultFormat = currentCharFormat();
    m_errorFormat = m_defaultFormat;
    m_errorFormat.setForeground(Qt::red);
    m_defaultFormat.setForeground(Qt::white);

    // Create process instance
    m_process = new QProcess(this);
    m_process->setProcessChannelMode(QProcess::MergedChannels);
    m_process->setReadChannel(QProcess::StandardOutput);



    // set up input handeler

    // Setup timers
    m_cursorBlinkTimer.setInterval(500);
    connect(&m_cursorBlinkTimer, &QTimer::timeout, this, &TerminalWidget::blinkCursor);

    m_resizeTimer.setInterval(100);
    m_resizeTimer.setSingleShot(true);
    connect(&m_resizeTimer, &QTimer::timeout, this, &TerminalWidget::sendResizeEvent);

    // Connect process signals
    connect(m_process, &QProcess::readyRead, this, &TerminalWidget::handleCommandOutput);
    connect(m_process, &QProcess::started, this, &TerminalWidget::onProcessStarted);
    connect(m_process, &QProcess::errorOccurred, this, &TerminalWidget::onProcessError);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &TerminalWidget::onProcessFinished);
}

void TerminalWidget::handleProcessFinished(int exitCode, QProcess::ExitStatus status)
{
    QString output = QString::fromLocal8Bit(m_process->readAllStandardOutput());
    QTextCursor cursor = this->textCursor();
    cursor.movePosition(QTextCursor::End);
    this->setTextCursor(cursor);
    cursor.insertText(output);

    qDebug() << "[Process finished] Code:" << exitCode << " Output:" << output;
}

void TerminalWidget::setInitialDirectory(const QString &directory) {
    if (QDir(directory).exists()) {
        // m_currentPath = QDir::toNativeSeparators(directory);
        m_currentPath=directory;
        m_state.currentDir = m_currentPath;
        setupTerminal();
        loadHistory();
        updatePrompt();
    } else {
        appendPlainText("Invalid directory path: " + directory + "\n");
    }
}


TerminalWidget::~TerminalWidget()
{
    // Clean up the process if it's still running
    if (m_process && m_process->state() == QProcess::Running) {
        m_process->terminate();
        if (!m_process->waitForFinished(500)) {  // Wait up to 500ms
            m_process->kill();  // Force kill if not terminating
            m_process->waitForFinished();
        }
    }

    // Delete the process object
    delete m_process;
    m_process = nullptr;

    // Save any pending history or settings
    saveHistory();
}
/*
void TerminalWidget::start()
{
    if (m_process->state() != QProcess::NotRunning) return;

    // Set up environment
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

#ifdef Q_OS_WIN
    // Add current directory to PATH on Windows
    env.insert("Path", env.value("Path") + ";" + m_state.currentDir);

    // Apply environment
    m_process->setProcessEnvironment(env);

    // Set working directory if provided
    if (!m_currentPath.isEmpty()) {
        m_process->setWorkingDirectory(m_currentPath);
    }

    // m_process->start("cmd.exe", QStringList() << "/Q" "/k");
    // m_process->start("cmd.exe", QStringList() << "/Q" "/k");
    m_process->start("powershell.exe", QStringList() << "-NoLogo" << "-NoExit");


#else
    // Set working directory on Linux/macOS
    if (!m_initialDir.isEmpty()) {
        m_process->setWorkingDirectory(m_initialDir);
    }

    m_process->setProcessEnvironment(env);
    m_process->start("/bin/bash", {"-i"});
#endif

    if (!m_process->waitForStarted(3000)) {
        appendPlainText("Failed to start terminal\n");
    }
}
*/
void TerminalWidget::start()
{
    if (m_process->state() != QProcess::NotRunning) return;

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    m_process->setProcessEnvironment(env);

    if (!m_currentPath.isEmpty()) {
        m_process->setWorkingDirectory(m_currentPath);
    }

#ifdef Q_OS_WIN
    // Start PowerShell instead of CMD
    m_process->start("powershell.exe", QStringList() << "-NoLogo" << "-NoExit");

#else
    // Linux/macOS
    if (!m_initialDir.isEmpty()) {
        m_process->setWorkingDirectory(m_initialDir);
    }
    m_process->start("/bin/bash", {"-i"});
#endif

    if (!m_process->waitForStarted(3000)) {
        appendPlainText("Failed to start terminal\n");
    }
}


void TerminalWidget::executeCommand(const QString &command)
{
    if (!m_process || m_process->state() != QProcess::Running) {
        appendPlainText("Terminal not ready\n");
        return;
    }

    QString cleanCmd = command.trimmed();
    if (cleanCmd.isEmpty()) return;

    // Convert Unix-style paths to Windows-style
    cleanCmd.replace("./", ".\\");
    // Command translation map
    static const QHash<QString, QString> unixToWinCommands = {
        {"ls", "dir"},
        {"clear", "cls"},
        {"pwd", "cd"},
        {"cp", "copy"},
        {"mv", "move"},
        {"rm", "del"}
    };

    // // Handle cd commands differently
    if (isChangingDirectory(cleanCmd)) {
        QString path = extractPathFromCdCommand(cleanCmd);
        m_pendingCommand = "cd";
        m_expectingPathResponse = true;
        QString command = QString("@cd /D \"%1\" && cd\r\n").arg(path);  // Prevents echo and prompt duplication
        m_process->write(command.toLocal8Bit());
        return;
    }
    // Translate Unix commands to Windows equivalents
#ifdef Q_OS_WIN
    QStringList parts = cleanCmd.split(' ', Qt::SkipEmptyParts);
    if (unixToWinCommands.contains(parts[0])) {
        parts[0] = unixToWinCommands[parts[0]];
        cleanCmd = parts.join(' ');
    }
#endif

    // Execute the command
    m_process->write(cleanCmd.toLocal8Bit() + "\r\n");
    m_lastCommand = cleanCmd;  // Store for output processing
}

/*
void TerminalWidget::handleCommandOutput()
{
    QByteArray data = m_process->readAll();
    QString output = QString::fromLocal8Bit(data);
    // // Process output line by line
    QStringList lines = output.split(QRegularExpression("[\r\n]"));
    for (const QString &line : lines) {
         //processCommandLine(line);
       processOutputLine(line);
    }

    updatePrompt();

}
*/
void TerminalWidget::handleCommandOutput()
{
    QByteArray data = m_process->readAll();
    QString output = QString::fromLocal8Bit(data);

    QStringList lines = output.split(QRegularExpression("[\r\n]"), Qt::SkipEmptyParts);
    for (const QString &line : lines) {
        processOutputLine(line);
    }

    updatePrompt();
}


void TerminalWidget::appendOutput(const QString &text) {
    // if (m_outputArea) {
    //     m_outputArea->moveCursor(QTextCursor::End);
    //     m_outputArea->insertPlainText(text);
    //     m_outputArea->moveCursor(QTextCursor::End);
    // }
    if (m_outputArea) {
        QTextCursor cursor = m_outputArea->textCursor();
        cursor.movePosition(QTextCursor::End);
        m_outputArea->setTextCursor(cursor);
        cursor.insertText(text);
    }
}


void TerminalWidget::processCommandLine(const QString &line)
{
    if (line.trimmed().isEmpty()) return;
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::End);

    // Check for path changes
    if (!m_pendingCommand.isEmpty() && m_pendingCommand.startsWith("cd ")) {
        QDir newDir(line);
        if (newDir.exists()) {
            m_currentPath = QDir::toNativeSeparators(newDir.absolutePath());
            m_state.currentDir = m_currentPath;
            return;
        }
    }
    if (line.trimmed().startsWith("cd /D")) return;
    // Check for errors
    if (line.contains("is not recognized") || line.contains("not found")) {
        cursor.setCharFormat(m_errorFormat);
        cursor.insertText(line);
        cursor.setCharFormat(m_defaultFormat);
    } else {
        cursor.insertText("\n");
        // Normal output
        cursor.setCharFormat(m_defaultFormat);
        cursor.insertText(line);
    }
}

void TerminalWidget::readProcessOutput()
{
    QByteArray data = m_process->readAll();
    QString output = QString::fromLocal8Bit(data);

    // Process each line
    QStringList lines = output.split(QRegularExpression("[\r\n]"), Qt::SkipEmptyParts);
    for (const QString &line : lines) {
        processOutputLine(line);
    }
}

/*
void TerminalWidget::processOutputLine(const QString &line)
{
    if (line.trimmed().isEmpty() || line == m_lastCommand || line == m_currentPath)
        return;

    // Avoid printing prompt again if it's already handled
    if (line.trimmed().endsWith(">"))
        return;


    if (m_expectingPathResponse && QDir(line.trimmed()).exists()) {
        m_currentPath = QDir::toNativeSeparators(line.trimmed());
        m_pendingCommand.clear();
        m_expectingPathResponse = false;
        //updatePrompt();
        return;
    }


    // Handle cd command errors
    if (m_expectingPathResponse &&
        (line.contains("not found") || line.contains("cannot find") || line.contains("The system cannot find"))) {
        appendPlainText(line + "\n");
        m_pendingCommand.clear();
        m_expectingPathResponse = false;
        //updatePrompt();  // Keep old path
        return;
    }


    // Process normal output
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::End);
    if (isErrorLine(line)) {
        cursor.insertText("\n");
        cursor.setCharFormat(m_errorFormat);
        cursor.insertText(line + "\n");
        cursor.setCharFormat(m_defaultFormat);
    } else {
        cursor.insertText("\n");
        cursor.setCharFormat(m_defaultFormat);
        cursor.insertText(line);
        //qDebug()<<cursor.document()->toPlainText();
    }
}

*/
void TerminalWidget::processOutputLine(const QString &line)
{
    QString trimmed = line.trimmed();

    if (trimmed.isEmpty() || trimmed == m_lastCommand)
        return;

    // 🧠 Skip repeating prompt-like lines
    if (trimmed == m_currentPath ||
        trimmed == m_currentPath + ">" ||
        trimmed.endsWith(">") || trimmed.contains(m_currentPath))
    {
        return;
    }

    // Handle successful 'cd' or path output
    if (m_expectingPathResponse && QDir(trimmed).exists()) {
        m_currentPath = QDir::toNativeSeparators(trimmed);
        m_pendingCommand.clear();
        m_expectingPathResponse = false;
        return;
    }

    // Handle directory errors
    if (m_expectingPathResponse &&
        (trimmed.contains("not found") ||
         trimmed.contains("cannot find") ||
         trimmed.contains("Set-Location") ||
         trimmed.contains("The system cannot find"))) {
        appendPlainText(trimmed + "\n");
        m_pendingCommand.clear();
        m_expectingPathResponse = false;
        return;
    }

    // Process normal output
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::End);

    if (isErrorLine(trimmed)) {
        cursor.insertText("\n");
        cursor.setCharFormat(m_errorFormat);
        cursor.insertText(trimmed + "\n");
        cursor.setCharFormat(m_defaultFormat);
    } else {
        cursor.insertText("\n");
        cursor.setCharFormat(m_defaultFormat);
        cursor.insertText(trimmed);
    }
}

bool TerminalWidget::isErrorLine(const QString &line)
{
    return line.contains("not recognized") ||
           line.contains("not found") ||
           line.contains("cannot find") ||
           line.contains("The system cannot find") ||
           line.contains("Set-Location") ||
           line.contains("Exception") ||
           line.contains("CategoryInfo");
}


/*
bool TerminalWidget::isErrorLine(const QString &line)
{
    return line.contains("not recognized") ||
           line.contains("not found") ||
           line.contains("cannot find") ||
           line.contains("The system cannot find");
}
*/


void TerminalWidget::updatePrompt()
{
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::End);

    // Remove existing prompt if present
    cursor.select(QTextCursor::LineUnderCursor);
    if (cursor.selectedText().endsWith(">")) {
        cursor.removeSelectedText();
    }

    // Insert new prompt
    cursor.setCharFormat(m_defaultFormat);
    cursor.insertText(m_currentPath + ">");
    ensureCursorVisible();
}





/*
void TerminalWidget::onProcessStarted()
{
#ifdef Q_OS_WIN
    m_process->write("@echo off\r\n");
    m_process->write("prompt $P$G\r\n");
#else
    m_process->write("PS1='\\w\\$ '\n");
    m_process->write("pwd\n");
    m_lastCommand = "pwd";  // So that output of 'pwd' is not printed again
#endif
    m_lastCommand.clear();
}
*/

void TerminalWidget::onProcessStarted()
{
#ifdef Q_OS_WIN
    QString quotedPath = "\"" + QDir::toNativeSeparators(m_currentPath) + "\"";
    m_process->write(("cd " + quotedPath + "\r\n").toLocal8Bit());

#else
    m_process->write("PS1='\\w\\$ '\n");
    m_process->write("pwd\n");
    m_lastCommand = "pwd";
#endif
    m_lastCommand.clear();
}


void TerminalWidget::onProcessError(QProcess::ProcessError error)
{
    Q_UNUSED(error);
    appendPlainText("Process error: " + m_process->errorString() + "\n");
    updatePrompt();
}

void TerminalWidget::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode);
    Q_UNUSED(exitStatus);
    appendPlainText("\nProcess finished. Type 'start' to restart.\n");
    updatePrompt();
}

void TerminalWidget::keyPressEvent(QKeyEvent *e)
{
    // Handle arrow keys for command history
    if (e->key() == Qt::Key_Up || e->key() == Qt::Key_Down) {
        handleArrowKeys(e);
        return;
    }

    // Handle backspace specially to prevent prompt deletion
    if (e->key() == Qt::Key_Backspace) {
        handleBackspace();
        return;
    }
    if (e->key() == Qt::Key_C && e->modifiers() == Qt::ControlModifier) {
        if (m_process && m_process->state() == QProcess::Running) {
            m_process->kill();
        }
        return;
    }
    if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) {
        handleEnterKey();
        return;
    }
    else {
        QPlainTextEdit::keyPressEvent(e);
    }
}

void TerminalWidget::sendResizeEvent()
{
    if (!m_process || m_process->state() != QProcess::Running) {
        return;
    }

#ifdef Q_OS_WIN
    QString columns = QString::number(width() / fontMetrics().averageCharWidth());
    QString lines = QString::number(height() / fontMetrics().height());
    m_process->write(("mode con: cols=" + columns + " lines=" + lines + "\r\n").toLocal8Bit());
#else
    struct winsize ws;
    ws.ws_row = static_cast<unsigned short>(height() / fontMetrics().height());
    ws.ws_col = static_cast<unsigned short>(width() / fontMetrics().averageCharWidth());

    if (m_process->processId() > 0) {
        ioctl(m_process->processId(), TIOCSWINSZ, &ws);
    }

    m_process->write(("stty rows " + QString::number(ws.ws_row) +
                      " cols " + QString::number(ws.ws_col) + "\n").toLocal8Bit());
#endif
}

void TerminalWidget::blinkCursor()
{
    m_state.showCursor = !m_state.showCursor;
    viewport()->update();
}

void TerminalWidget::paintEvent(QPaintEvent *e)
{
    QPlainTextEdit::paintEvent(e);

        // Draw blinking cursor
        if (hasFocus() && m_state.showCursor) {
            QPainter painter(viewport());
            QRect cursorRect = this->cursorRect();
            painter.fillRect(cursorRect, palette().color(QPalette::Text));
        }


}

// ... (implement other methods like setupTerminal, loadHistory, saveHistory) ...

void TerminalWidget::setupTerminal()
{
    // Initialize terminal state
    // m_state.currentDir = QDir::currentPath();
    m_state.currentDir=m_currentPath;
    m_state.hostName = QSysInfo::machineHostName();
    m_state.userName = qgetenv("USER");
    if (m_state.userName.isEmpty())
        m_state.userName = qgetenv("USERNAME");
}

// Load command history
void TerminalWidget::loadHistory()
{
    QSettings settings;
    m_commandHistory = settings.value("terminal/history").toStringList();
    m_historyIndex = m_commandHistory.size();
}

// Save command history
void TerminalWidget::saveHistory()
{
    QSettings settings;
    settings.setValue("terminal/history", m_commandHistory);
}


// key event handeling
void TerminalWidget::handleArrowKeys(QKeyEvent *e)
{
    if (m_commandHistory.isEmpty()) return;

    QTextCursor cursor = textCursor();
    cursor.select(QTextCursor::LineUnderCursor);
    QString currentLine = cursor.selectedText();

    // Only allow history navigation at the current prompt
    if (!currentLine.startsWith(m_currentPath + ">")) {
        return;
    }

    // Navigate history
    if (e->key() == Qt::Key_Up && m_historyIndex > 0) {
        m_historyIndex--;
    } else if (e->key() == Qt::Key_Down && m_historyIndex < m_commandHistory.size() - 1) {
        m_historyIndex++;
    } else {
        return;
    }

    // Replace current line with history item
    cursor.removeSelectedText();
    cursor.insertText(m_currentPath + ">" + m_commandHistory.at(m_historyIndex));
}

void TerminalWidget::handleBackspace()
{
    QTextCursor cursor = textCursor();
    int promptEndPos = getPromptPosition();

    // Only allow backspace if we're after the prompt
    if (cursor.positionInBlock() > promptEndPos) {
        cursor.deletePreviousChar();
    }
    // Optional: Add a beep sound when trying to delete prompt
    else {
        QApplication::beep();
    }
}

int TerminalWidget::getPromptPosition()
{
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::StartOfLine);

    // Get the current line text
    cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
    QString line = cursor.selectedText();
    cursor.clearSelection();

    // Find the end of prompt (">" character)
    int promptEnd = line.indexOf('>');
    if (promptEnd >= 0) {
        return promptEnd + 1; // Position right after ">"
    }

    // Fallback if no prompt found
    return cursor.positionInBlock(); // Current position
}

void TerminalWidget::restoreCurrentLine(const QString &text)
{
    QTextCursor cursor = textCursor();
    cursor.select(QTextCursor::LineUnderCursor);
    cursor.removeSelectedText();
    cursor.insertText(m_currentPath + ">" + text);
}


void TerminalWidget::handleEnterKey()
{
    QTextCursor cursor = textCursor();
    cursor.select(QTextCursor::LineUnderCursor);
    QString fullLine = cursor.selectedText();

    // Extract command (after prompt)
    QString command = fullLine.mid(fullLine.indexOf('>') + 1).trimmed();

    if (!command.isEmpty()) {
        // Add to history if not duplicate
        if (m_commandHistory.empty() || m_commandHistory.last() != command) {
            m_commandHistory.append(command);
        }
        m_historyIndex = m_commandHistory.size();

        // Execute command
        executeCommand(command);
    }

    // Add new prompt
    appendPlainText("\n" + m_currentPath + ">");
}


void TerminalWidget::appendText(const QString &text) {
    moveCursor(QTextCursor::End);
    insertPlainText(text);
    ensureCursorVisible();
    promptPosition = textCursor().position();
}
