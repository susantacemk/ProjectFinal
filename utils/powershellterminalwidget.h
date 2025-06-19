/*#ifndef POWERSHELLTERMINALWIDGET_H
#define POWERSHELLTERMINALWIDGET_H

class PowerShellTerminalWidget
{
public:
    PowerShellTerminalWidget();
};

#endif */// POWERSHELLTERMINALWIDGET_H


#ifndef POWERSHELLTERMINALWIDGET_H
#define POWERSHELLTERMINALWIDGET_H

#include <QPlainTextEdit>
#include <QProcess>
#include <QTimer>
#include <QTextCharFormat>

class PowerShellTerminalWidget : public QPlainTextEdit {
    Q_OBJECT

public:
    explicit PowerShellTerminalWidget(QWidget *parent = nullptr);
    ~PowerShellTerminalWidget();

    void setInitialDirectory(const QString &path);
    void startTerminal();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void handleOutput();
    //void onProcessStarted();
    bool isPromptEcho(const QString &line) const;
    void handleErrorOutput();

private:
    void executeCommand(const QString &command);
    void processOutputLine(const QString &line);
    void updatePrompt();

    QProcess *m_process;
    QString m_currentDir;
    QString m_lastCommand;

    QTextCharFormat m_defaultFormat;
    QTextCharFormat m_errorFormat;

    QString promptText() const;
    bool isPromptLine(const QString &line) const;
    bool isErrorLine(const QString &line) const;



    int m_promptPosition = 0;
    void insertPrompt();

    bool m_waitingForInput = false;

    QStringList m_commandHistory;
    int m_historyIndex;
    void replaceCurrentCommand(const QString &command);
    void handleTabCompletion();

    QTextCharFormat m_promptFormat;
    QTextCharFormat m_outputFormat;


    QStringList m_availableCommands = { "clear", "exit", "ls", "cd", "gcc", "g++", "dir", "cls", "echo" };


};

#endif // POWERSHELLTERMINALWIDGET_H
