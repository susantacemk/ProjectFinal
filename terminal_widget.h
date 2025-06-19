
#ifndef TERMINAL_WIDGET_H
#define TERMINAL_WIDGET_H

#include <QPlainTextEdit>
#include<QRegularExpression>
#include <QProcess>
#include <QTimer>
#include <QPointer>
#include "ansiparser.h"  // Custom ANSI parser (provided below)

class TerminalWidget : public QPlainTextEdit {
    Q_OBJECT
public:
    enum TerminalMode {
        DefaultMode,
        ViMode,
        EmacsMode
    };

    explicit TerminalWidget(QWidget *parent = nullptr, TerminalMode mode = DefaultMode);
    ~TerminalWidget();

    //void start(const QString &program = QString(), const QStringList &args = QStringList());
    void start();
    void executeCommand(const QString &command);
    void setWorkingDirectory(const QString &dir);
    void setTerminalMode(TerminalMode mode);
    void setScrollbackLimit(int lines);
    void copySelection();
    void pasteClipboard();
    void clearTerminal();
    void setAutoScroll(bool enable);
    bool autoScroll() const;
    void sendSignal(int signal);  // For UNIX signals
    QString translateCommand(const QString &command);
    void updateWorkingDirectory();
    //void readOutput();
    //void readError();
    void sendResize();
    void setInitialDirectory(const QString &directory);
    void appendText(const QString &text);
    void stopProgram();
    QString getLastLineText();
    bool isProcessRunning() const;

signals:
    void titleChanged(const QString &title);
    void currentDirectoryChanged(const QString &dir);
    //void processFinished(int exitCode);

protected:
    void keyPressEvent(QKeyEvent *e) override;
    //void mousePressEvent(QMouseEvent *e) override;
    //void mouseDoubleClickEvent(QMouseEvent *e) override;
    //void contextMenuEvent(QContextMenuEvent *e) override;
    //void resizeEvent(QResizeEvent *e) override;
    void paintEvent(QPaintEvent *e) override;

private slots:
    // void readFromProcess();
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    //void updateTerminalSize();
    void blinkCursor();
    void updatePrompt();
    void handleCommandOutput();
    void handleProcessFinished(int exitCode, QProcess::ExitStatus status);
private:

     QPlainTextEdit *m_outputArea;

    bool isChangingDirectory(const QString& command) {
        return command.startsWith("cd ", Qt::CaseInsensitive);
    }

    QString extractPathFromCdCommand(const QString& command) {
        return command.mid(3).trimmed().replace("/", "\\");
    }
    bool m_autoScroll = true;
    bool m_expectingPathUpdate = false;
    bool m_expectingPathResponse = false;
    QString m_lastCdCommand;
    struct TerminalState {
        QString currentDir;
        QString hostName;
        QString userName;
        QString lastCommand;
        int cursorX = 0;
        int cursorY = 0;
        bool showCursor = true;
    };

    QPointer<QProcess> m_process;
    TerminalState m_state;
    TerminalMode m_mode;
    AnsiParser m_ansiParser;
    QTimer m_cursorBlinkTimer;
    QTimer m_resizeTimer;
    int m_scrollbackLimit = 10000;
    bool m_altKeyPressed = false;
    bool m_hasSelection = false;
    QString m_selectedText;
    QStringList m_commandHistory;
    int m_historyIndex = 0;

    void startExecutableSubprocess(const QString &cmd);
    void setupTerminal();
    void handleSpecialKeys(QKeyEvent *e);
    void handleViKeys(QKeyEvent *e);
    void handleEmacsKeys(QKeyEvent *e);
    void updateWindowTitle();
    void appendPrompt();
    void processOutput();
    void applyAnsiFormatting(const QString &text);
    void sendResizeEvent();
    void updateSelection();
    void saveHistory();
    void loadHistory();
    void trimScrollback();
    void startProcessForExecutable(const QString &cmd);
    void onSendInputToProcess(const QString &input);
    QString getShellProgram() const;
    const QStringList getShellArguments() const;
    void initializeEnvironment();
    void setupSignals();
    QString m_lastCommand;
    QString currentPrompt() const;
    QTextCharFormat m_defaultFormat;
    QTextCharFormat m_errorFormat;
    QString m_currentPath;
    void updateCurrentPath();

    void appendOutput(const QString &text);

    void handleArrowKeys(QKeyEvent *e);
    void handleBackspace();
    void handleEnterKey();
    void restoreCurrentLine(const QString &text);
    int getPromptPosition();

    // QTextCharFormat m_defaultFormat;
    // QTextCharFormat m_errorFormat;
    // QString m_currentPath;
    QString m_pendingCommand;
    bool m_processingOutput = false;
    // void updateCurrentPath();
    void processCommandOutput(const QString& output);
    void processCommandLine(const QString &line);

    void onProcessError(QProcess::ProcessError error);
    void onProcessStarted();
    // void processFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void processOutputLine(const QString &line);
    void readProcessOutput();
    void initializeShell();
    bool isErrorLine(const QString &line);

    QString m_lastPrintedLine;
    QProcess *m_execProcess = nullptr;
    bool m_waitingForInput = false;


    // new - version
    QProcess *process = nullptr;
    int promptPosition = 0;

};
#endif // TERMINAL_WIDGET_H

/*
#ifndef TERMINAL_WIDGET_H
#define TERMINAL_WIDGET_H

#include <QPlainTextEdit>
#include <QProcess>

class TerminalWidget : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit TerminalWidget(QWidget *parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    QProcess *m_process;
    QProcess *m_subProcess;
    bool m_executableRunning;
    int m_promptPosition;

    void appendTextWithPrompt(const QString &text);
    QString currentPrompt() const;

    void handleUserInput(const QString &input);
    void executeCommand(const QString &command);
    void startExecutableSubprocess(const QString &path);
};

#endif // TERMINAL_WIDGET_H

*/

