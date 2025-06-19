// #ifndef LSPCLIENTPYTHON_H
// #define LSPCLIENTPYTHON_H

// class LSPClientPython
// {
// public:
//     LSPClientPython();
// };

// #endif // LSPCLIENTPYTHON_H

#pragma once

#include <QObject>
#include <QProcess>
#include <QJsonObject>

class LSPClientPython : public QObject {
    Q_OBJECT

public:
    explicit LSPClientPython(QObject *parent = nullptr);
    void start(const QString &projectPath);
    void sendDidOpen(const QString &uri, const QString &content);
    void sendDidChange(const QString &uri, const QString &content, int version);
    void sendCompletionRequest(const QString &uri, int line, int character);

signals:
    void completionsReceived(const QStringList &suggestions);
    void diagnosticsReceived(const QJsonArray &diagnostics);

private slots:
    void readFromServer();

private:
    QProcess *process = nullptr;
    QByteArray buffer;
    int requestId = 1;

    void sendMessage(const QJsonObject &json);
};
