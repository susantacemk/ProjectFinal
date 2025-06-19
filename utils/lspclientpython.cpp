#include "lspclientpython.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QCoreApplication>


LSPClientPython::LSPClientPython(QObject *parent) : QObject(parent) {
    process = new QProcess(this);
    connect(process, &QProcess::readyReadStandardOutput, this, &LSPClientPython::readFromServer);
}

void LSPClientPython::start(const QString &projectPath) {
    process->start("npx", QStringList() << "pyright-langserver" << "--stdio");

    QJsonObject params;
    params["processId"] = QCoreApplication::applicationPid();
    params["rootUri"] = "file://" + projectPath;
    params["capabilities"] = QJsonObject();  // empty object

    QJsonObject init;
    init["jsonrpc"] = "2.0";
    init["id"] = requestId++;
    init["method"] = "initialize";
    init["params"] = params;

    sendMessage(init);

}

void LSPClientPython::sendDidOpen(const QString &uri, const QString &content) {
    QJsonObject textDocument;
    textDocument["uri"] = uri;
    textDocument["languageId"] = "python";
    textDocument["version"] = 1;
    textDocument["text"] = content;

    QJsonObject params;
    params["textDocument"] = textDocument;

    QJsonObject msg;
    msg["jsonrpc"] = "2.0";
    msg["method"] = "textDocument/didOpen";
    msg["params"] = params;

    sendMessage(msg);

}

void LSPClientPython::sendDidChange(const QString &uri, const QString &content, int version) {
    QJsonObject textDocument;
    textDocument["uri"] = uri;
    textDocument["version"] = version;

    QJsonObject change;
    change["text"] = content;

    QJsonArray changes;
    changes.append(change);

    QJsonObject params;
    params["textDocument"] = textDocument;
    params["contentChanges"] = changes;

    QJsonObject msg;
    msg["jsonrpc"] = "2.0";
    msg["method"] = "textDocument/didChange";
    msg["params"] = params;

    sendMessage(msg);

}

void LSPClientPython::sendCompletionRequest(const QString &uri, int line, int character) {
    QJsonObject textDocument;
    textDocument["uri"] = uri;

    QJsonObject position;
    position["line"] = line;
    position["character"] = character;

    QJsonObject params;
    params["textDocument"] = textDocument;
    params["position"] = position;

    QJsonObject msg;
    msg["jsonrpc"] = "2.0";
    msg["id"] = requestId++;
    msg["method"] = "textDocument/completion";
    msg["params"] = params;

    sendMessage(msg);

}

void LSPClientPython::sendMessage(const QJsonObject &json) {
    QJsonDocument doc(json);
    QByteArray body = doc.toJson(QJsonDocument::Compact);
    QByteArray header = "Content-Length: " + QByteArray::number(body.length()) + "\r\n\r\n";
    process->write(header + body);
    // process->flush();
}

void LSPClientPython::readFromServer() {
    buffer += process->readAllStandardOutput();

    while (true) {
        int headerEnd = buffer.indexOf("\r\n\r\n");
        if (headerEnd == -1) return;

        QByteArray header = buffer.left(headerEnd);
        buffer = buffer.mid(headerEnd + 4);

        int contentLength = 0;
        for (const QByteArray &line : header.split('\n')) {
            if (line.startsWith("Content-Length:")) {
                contentLength = line.mid(15).trimmed().toInt();
                break;
            }
        }

        if (buffer.length() < contentLength) return;

        QByteArray body = buffer.left(contentLength);
        buffer = buffer.mid(contentLength);

        QJsonDocument doc = QJsonDocument::fromJson(body);
        QJsonObject obj = doc.object();

        if (obj.contains("method") && obj["method"] == "textDocument/publishDiagnostics") {
            QJsonObject params = obj["params"].toObject();
            emit diagnosticsReceived(params["diagnostics"].toArray());
        }

        if (obj.contains("id") && obj.contains("result")) {
            QJsonArray items = obj["result"].toObject()["items"].toArray();
            QStringList suggestions;
            for (const QJsonValue &v : items)
                suggestions << v.toObject()["label"].toString();
            emit completionsReceived(suggestions);
        }
    }
}

