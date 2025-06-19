#ifndef COMPILATIONMANAGER_H
#define COMPILATIONMANAGER_H
#include<QString>
#include<QObject>

// compilationmanager.h
class CompilationManager : public QObject {
    Q_OBJECT
public:
    explicit CompilationManager(QObject *parent = nullptr);

    void compileFile(const QString &filePath);

    void compileFile1(const QString &filePath);

signals:
    void compilationOutput(const QString &text);
    void compilationFinished(bool success);
    void compilationFinished1(bool success, const QString &outputFilePath, const QString &errors);
};

#endif // COMPILATIONMANAGER_H
