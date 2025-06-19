#include "compilationmanager.h"
#include <QFileInfo>
#include<QProcess>
CompilationManager::CompilationManager(QObject *parent) : QObject(parent) {}

void CompilationManager::compileFile(const QString &filePath) {
    QFileInfo fileInfo(filePath);
    QString extension = fileInfo.suffix().toLower();
    QString compiler, outputFile = fileInfo.completeBaseName() + ".out";
    QStringList arguments;

    if (extension == "c") {
        compiler = "gcc";
        arguments << filePath << "-o" << outputFile;
    } else if (extension == "cpp") {
        compiler = "g++";
        arguments << filePath << "-o" << outputFile;
    } else if (extension == "py") {
        // Python syntax check only (compilation-like check)
        compiler = "python";
        arguments << "-m" << "py_compile" << filePath;
    } else {
        emit compilationOutput("❌ Unsupported file type for compilation.\n");
        emit compilationFinished(false);
        return;
    }

    QProcess *process = new QProcess(this);

    connect(process, &QProcess::readyReadStandardError, [=]() {
        QString error = process->readAllStandardError();
        emit compilationOutput(error);
    });

    connect(process, &QProcess::readyReadStandardOutput, [=]() {
        QString output = process->readAllStandardOutput();
        emit compilationOutput(output);
    });

    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [=](int exitCode, QProcess::ExitStatus) {
                if (exitCode == 0) {
                    emit compilationOutput("✅ Compiled successfully without error.\n");
                    emit compilationFinished(true);
                } else {
                    emit compilationOutput("❌ Compilation failed with errors.\n");
                    emit compilationFinished(false);
                }
                process->deleteLater();
            });

    process->start(compiler, arguments);
}

void CompilationManager::compileFile1(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    QString extension = fileInfo.suffix().toLower();
    QString compiler, outputFile;
    QStringList arguments;

#ifdef Q_OS_WIN
    QString outExtension = ".exe";
#else
    QString outExtension = ".out";
#endif

    outputFile = fileInfo.absolutePath() + "/" + fileInfo.completeBaseName() + outExtension;

    if (extension == "c") {
        compiler = "gcc";
        arguments << filePath << "-o" << outputFile;
    } else if (extension == "cpp") {
        compiler = "g++";
        arguments << filePath << "-o" << outputFile;
    } else if (extension == "py") {
        // Python doesn't compile to binary, just check syntax
        compiler = "python";
        arguments << "-m" << "py_compile" << filePath;
        outputFile = "";  // No binary output
    } else {
        emit compilationOutput("❌ Unsupported file type for compilation.\n");
        emit compilationFinished1(false, "", "Unsupported file type");
        return;
    }

    QProcess *process = new QProcess(this);

    QString allOutput;

    connect(process, &QProcess::readyReadStandardError, [=, &allOutput]() mutable {
        QString error = process->readAllStandardError();
        allOutput += error;
        emit compilationOutput(error);
    });

    connect(process, &QProcess::readyReadStandardOutput, [=, &allOutput]() mutable {
        QString output = process->readAllStandardOutput();
        allOutput += output;
        emit compilationOutput(output);
    });

    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [=, &allOutput](int exitCode, QProcess::ExitStatus) mutable {
                if (exitCode == 0) {
                    emit compilationOutput("✅ Compiled successfully without error.\n");
                    emit compilationFinished1(true, outputFile, "");
                } else {
                    emit compilationOutput("❌ Compilation failed.\n");
                    emit compilationFinished1(false, "", allOutput.trimmed());
                }
                process->deleteLater();
            });

    process->start(compiler, arguments);
}


