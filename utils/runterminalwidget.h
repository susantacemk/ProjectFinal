// #ifndef RUNTERMINALWIDGET_H
// #define RUNTERMINALWIDGET_H

// class RunTerminalWidget
// {
// public:
//     RunTerminalWidget();
// };

// #endif // RUNTERMINALWIDGET_H


#ifndef RUNTERMINALWIDGET_H
#define RUNTERMINALWIDGET_H

#include <QWidget>
#include <QProcess>
#include <QPlainTextEdit>
#include <QLineEdit>
#include <QVBoxLayout>

class RunTerminalWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RunTerminalWidget(QWidget *parent = nullptr);
    // void startProgram(const QString &executablePath);
    // In runterminalwidget.h
    void startProgram(const QString &program, const QStringList &args = QStringList());
    void killProgram();
    bool isRunning() const;
    void appendOutput(const QString &text);
    void compileBeforeRun(const QString &filePath);
private slots:
    void handleStandardOutput();
    void handleStandardError();
    void handleProcessFinished();
    void sendUserInput();

private:
    QProcess *m_process;
    QPlainTextEdit *outputArea;
    QLineEdit *inputField;
};

#endif // RUNTERMINALWIDGET_H
