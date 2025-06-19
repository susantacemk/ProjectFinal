#include "runterminalwidget.h"
#include <QFileInfo>
#include <QScrollBar>

RunTerminalWidget::RunTerminalWidget(QWidget *parent)
    : QWidget(parent), m_process(new QProcess(this))
{
    outputArea = new QPlainTextEdit(this);
    outputArea->setReadOnly(true);
    outputArea->setStyleSheet("background-color: #1e1e1e; color: white; font-family: Consolas;");

    inputField = new QLineEdit(this);
    inputField->setPlaceholderText("Enter input...");
    inputField->hide();

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(outputArea);
    layout->addWidget(inputField);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    QFont terminalFont("Consolas");
    terminalFont.setStyleHint(QFont::Monospace);
    terminalFont.setPointSize(11);

    outputArea->setFont(terminalFont);
    outputArea->setStyleSheet("background-color: #1e1e1e; color: white;");

    inputField->setFont(terminalFont);
    inputField->setStyleSheet("color: white; background-color: #2e2e2e;");

    connect(inputField, &QLineEdit::returnPressed, this, &RunTerminalWidget::sendUserInput);
    connect(m_process, &QProcess::readyReadStandardOutput, this, &RunTerminalWidget::handleStandardOutput);
    connect(m_process, &QProcess::readyReadStandardError, this, &RunTerminalWidget::handleStandardError);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &RunTerminalWidget::handleProcessFinished);
}
/*
void RunTerminalWidget::startProgram(const QString &executablePath)
{
    if (m_process->state() != QProcess::NotRunning) {
        m_process->kill();
        m_process->waitForFinished(1000); // wait up to 1 second for cleanup
    }

    outputArea->clear();
    inputField->clear();
    inputField->hide();

#ifdef Q_OS_WIN
    QString executable = executablePath.endsWith(".exe") ? executablePath : executablePath + ".exe";
#else
    QString executable = executablePath;
#endif

    m_process->setProgram(executable);
    m_process->setWorkingDirectory(QFileInfo(executable).absolutePath());
    m_process->start();

    outputArea->appendPlainText("▶ Running: " + executable);
}

*/
void RunTerminalWidget::startProgram(const QString &program, const QStringList &args)
{
    if (m_process && m_process->state() == QProcess::Running) {
        m_process->kill();
        m_process->waitForFinished();
    }
    outputArea->clear();
    inputField->clear();
    inputField->hide();

    m_process = new QProcess(this);
    m_process->setProgram(program);
    m_process->setArguments(args);
    m_process->setProcessChannelMode(QProcess::MergedChannels);

    connect(m_process, &QProcess::readyReadStandardOutput,
            this, &RunTerminalWidget::handleStandardOutput);


    connect(m_process, &QProcess::readyReadStandardError,
            this, &RunTerminalWidget::handleStandardError);


    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [=](int, QProcess::ExitStatus) {
                appendOutput("\n✅ Program finished.\n");
                inputField->hide();
                m_process->deleteLater();
                m_process = nullptr;
            });

    m_process->start();
}

void RunTerminalWidget::handleStandardOutput()
{
    QString text = QString::fromUtf8(m_process->readAllStandardOutput());
    outputArea->appendPlainText(text.trimmed());
    outputArea->verticalScrollBar()->setValue(outputArea->verticalScrollBar()->maximum());

    if (m_process->state() == QProcess::Running) {
        inputField->setEnabled(true);
        inputField->show();
        inputField->setFocus();
    }
}
/*
void RunTerminalWidget::handleStandardOutput()
{
    QString text = QString::fromUtf8(m_process->readAllStandardOutput());
    outputArea->appendPlainText(text.trimmed());
    outputArea->verticalScrollBar()->setValue(outputArea->verticalScrollBar()->maximum());

    // Show input only if output seems like it's prompting for input
    if (m_process->state() == QProcess::Running &&
        (text.contains("input") || text.contains(":") || text.contains("?"))) {
        inputField->setEnabled(true);
        inputField->show();
        inputField->setFocus();
    }
}*/

void RunTerminalWidget::handleStandardError()
{
    QString text = QString::fromUtf8(m_process->readAllStandardError());
    outputArea->appendPlainText("[stderr]: " + text.trimmed());
    outputArea->verticalScrollBar()->setValue(outputArea->verticalScrollBar()->maximum());
}

void RunTerminalWidget::sendUserInput()
{
    if (m_process && m_process->state() == QProcess::Running) {
        QString input = inputField->text();
        m_process->write(input.toUtf8() + "\n");

        outputArea->appendPlainText(input); // Optional echo
        inputField->clear();
        inputField->setEnabled(false);
    }
}

void RunTerminalWidget::handleProcessFinished()
{
    outputArea->appendPlainText("\n[Process Finished]");
    inputField->hide();
}

void RunTerminalWidget::killProgram()
{
    if (m_process && m_process->state() != QProcess::NotRunning) {
        m_process->kill();
    }
}

bool RunTerminalWidget::isRunning() const
{
    return m_process && m_process->state() == QProcess::Running;
}

void RunTerminalWidget::appendOutput(const QString &text) {
    outputArea->appendPlainText(text);
}

/*
void RunTerminalWidget::compileBeforeRun(const QString &filePath)
{
    // Kill old process if still running
    if (m_process && m_process->state() == QProcess::Running) {
        m_process->kill();        // forcibly stop
        m_process->waitForFinished(); // ensure it's really terminated
    }

    QFileInfo info(filePath);
    QString extension = info.suffix().toLower();
    QString outputPath = info.absolutePath() + "/" + info.completeBaseName();

#ifdef Q_OS_WIN
    outputPath += ".exe";
#else
    outputPath += ".out";
#endif

    QString compiler;
    QStringList arguments;

    if (extension == "c") {
        compiler = "gcc";
        arguments << filePath << "-o" << outputPath;
    } else if (extension == "cpp") {
        compiler = "g++";
        arguments << filePath << "-o" << outputPath;
    } else {
        appendOutput("❌ Unsupported file type for compilation.\n");
        return;
    }

    QProcess *compileProcess = new QProcess(this);

    connect(compileProcess, &QProcess::readyReadStandardError, [=]() {
        appendOutput(QString::fromUtf8(compileProcess->readAllStandardError()));
    });

    connect(compileProcess, &QProcess::readyReadStandardOutput, [=]() {
        appendOutput(QString::fromUtf8(compileProcess->readAllStandardOutput()));
    });

    connect(compileProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [=](int exitCode, QProcess::ExitStatus) {
                if (exitCode == 0) {
                    appendOutput("✅ Compilation successful.\n▶ Running: " + outputPath + "\n");
                    startProgram(outputPath);
                } else {
                    appendOutput("❌ Compilation failed.\n");
                }
                compileProcess->deleteLater();
            });

    appendOutput("🔧 Compiling: " + filePath + "\n");
    compileProcess->start(compiler, arguments);
}
*/
void RunTerminalWidget::compileBeforeRun(const QString &filePath)
{


    if (m_process && m_process->state() == QProcess::Running) {
        m_process->kill();
        m_process->waitForFinished();
    }

    QFileInfo info(filePath);
    QString extension = info.suffix().toLower();

    if (extension == "py") {
        // No compilation, directly run with Python
        appendOutput("▶ Running Python: " + filePath + "\n");
        startProgram("python", QStringList() << filePath);
        return;
    }

    // C/C++ Compilation
    QString outputPath = info.absolutePath() + "/" + info.completeBaseName();
#ifdef Q_OS_WIN
    outputPath += ".exe";
#else
    outputPath += ".out";
#endif

    QString compiler;
    QStringList arguments;

    if (extension == "c") {
        compiler = "gcc";
        arguments << filePath << "-o" << outputPath;
    } else if (extension == "cpp") {
        compiler = "g++";
        arguments << filePath << "-o" << outputPath;
    } else {
        appendOutput("❌ Unsupported file type.\n");
        return;
    }

    QProcess *compileProcess = new QProcess(this);

    connect(compileProcess, &QProcess::readyReadStandardError, [=]() {
        appendOutput(compileProcess->readAllStandardError());
    });

    connect(compileProcess, &QProcess::readyReadStandardOutput, [=]() {
        appendOutput(compileProcess->readAllStandardOutput());
    });

    connect(compileProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [=](int exitCode, QProcess::ExitStatus) {
                if (exitCode == 0) {
                    appendOutput("✅ Compilation successful.\n▶ Running: " + outputPath + "\n");
                    startProgram(outputPath);
                } else {
                    appendOutput("❌ Compilation failed.\n");
                }
                compileProcess->deleteLater();
            });

    appendOutput("🔧 Compiling: " + filePath + "\n");
    compileProcess->start(compiler, arguments);
}
