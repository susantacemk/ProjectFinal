#include "lspclient.h"
#include <QJsonDocument>
#include <QDebug>
#include<QCoreApplication>
#include<QJsonObject>
#include<QJsonArray>
#include<QJsonValue>
#include<QStringList>
LSPClient::LSPClient(QObject *parent) : QObject(parent) {
    connect(&process, &QProcess::readyReadStandardOutput, this, &LSPClient::readFromServer);
}

LSPClient:: ~LSPClient(){

}

void LSPClient::start(const QString &rootPath) {
   QString clangdPath = QCoreApplication::applicationDirPath() + "/clangd-mingw/bin/clangd.exe";
    QStringList args = {
        "--log=verbose",
        "--pretty"
    };

    process.setProgram(clangdPath);
    process.setArguments(args);
    process.start();


    if (!process.waitForStarted()) {
        qWarning() << "clangd not started";
        qWarning() << "clangd error:" << process.errorString();
        //qWarning() << "clangd full path:" << clangdPath;
        return;
    }

    QJsonObject capabilities;
    capabilities["textDocument"] = QJsonObject{
        {"documentSymbol", QJsonObject{
                               {"hierarchicalDocumentSymbolSupport", true}
                           }}
    };

    QJsonObject params{
        {"processId", QCoreApplication::applicationPid()},
        {"rootUri", "file://" + rootPath},
        {"capabilities", capabilities}
    };

    QJsonObject initialize{
        {"jsonrpc", "2.0"},
        {"id", int(nextRequestId++)},
        {"method", "initialize"},
        {"params", params}
    };

    pendingRequests[initialize["id"].toInt()] = "initialize";

    sendMessage(initialize);
}


/*
void LSPClient::start(const QString &rootPath) {
    process.start("clangd", QStringList() << "--log=verbose" << "--pretty");
    if (!process.waitForStarted()) {
        qWarning() << "clangd not started";
        return;
    }

    // ✅ Enable hierarchicalDocumentSymbolSupport
    QJsonObject capabilities;
    capabilities["textDocument"] = QJsonObject{
        {"documentSymbol", QJsonObject{
                               {"hierarchicalDocumentSymbolSupport", true}
                           }}
    };

    QJsonObject params{
        {"processId", QCoreApplication::applicationPid()},
        {"rootUri", "file://" + rootPath},
        {"capabilities", capabilities}
    };

    QJsonObject initialize{
        {"jsonrpc", "2.0"},
        {"id", int(nextRequestId++)},
        {"method", "initialize"},
        {"params", params}
    };

    // Track this request to filter later in readFromServer
    pendingRequests[initialize["id"].toInt()] = "initialize";

    sendMessage(initialize);
}
*/
void LSPClient::sendMessage(const QJsonObject &msg) {
    QJsonDocument doc(msg);
    QByteArray data = doc.toJson(QJsonDocument::Compact);
    QByteArray header = "Content-Length: " + QByteArray::number(data.size()) + "\r\n\r\n";
    process.write(header + data);
}

void LSPClient::readFromServer() {
    buffer.append(process.readAllStandardOutput());

    while (true) {
        int headerEnd = buffer.indexOf("\r\n\r\n");
        if (headerEnd == -1) return;

        QByteArray header = buffer.left(headerEnd);
        buffer.remove(0, headerEnd + 4);

        int contentLength = 0;
        for (const QByteArray &line : header.split('\n')) {
            if (line.startsWith("Content-Length:")) {
                contentLength = line.mid(15).trimmed().toInt();
                break;
            }
        }

        if (buffer.size() < contentLength) return;

        QByteArray content = buffer.left(contentLength);
        buffer.remove(0, contentLength);

        QJsonDocument doc = QJsonDocument::fromJson(content);
        if (!doc.isObject()) continue;

        QJsonObject obj = doc.object();

        // 🔍 Optional: Uncomment to inspect every raw LSP message
        // qDebug() << "📨 LSP Message:\n" << doc.toJson(QJsonDocument::Indented);

        //  1. Handle diagnostics
        if (obj.contains("method") && obj["method"].toString() == "textDocument/publishDiagnostics") {
            QJsonArray diagnostics = obj["params"].toObject()["diagnostics"].toArray();
            emit diagnosticsReceived(diagnostics);
            continue;
        }

        // ❗️2. Handle server error
        if (obj.contains("error")) {
            //qWarning() << "LSP Error:\n" << QJsonDocument(obj).toJson(QJsonDocument::Indented);
            continue;
        }

        // 🔁 3. Handle responses with an ID
        if (obj.contains("id")) {
            int id = obj["id"].toInt();

            // Identify the request type
            QString requestType = pendingRequests.take(id);

            if (!obj.contains("result")) {
                qWarning() << "Response missing 'result':" << QJsonDocument(obj).toJson(QJsonDocument::Indented);
                continue;
            }

            QJsonValue result = obj["result"];

            if (requestType == "completion") {
                QStringList suggestions;
                if (result.isObject()) {
                    QJsonObject resultObj = result.toObject();
                    if (resultObj.contains("items") && resultObj["items"].isArray()) {
                        QJsonArray items = resultObj["items"].toArray();
                        for (const QJsonValue &item : items) {
                            suggestions << item.toObject()["label"].toString();
                        }
                    }
                }
                if (!suggestions.isEmpty()) {
                    emit completionsReceived(suggestions);
                }
            }

            else if (requestType == "symbols") {
                if (result.isArray()) {
                    emit documentSymbolsReceived(result.toArray());
                } else {
                    qWarning() << "Unexpected symbol result format:" << result;
                }
            }

            else if (requestType == "rename") {
                //qDebug() << "✅ Rename result received.";
                handleRenameResult(result.toObject());
            }

            else {
                qWarning() << "Unknown requestType:" << requestType;
                //qDebug() << "Raw LSP response:" << QJsonDocument(obj).toJson(QJsonDocument::Indented);
            }
        }
    }
}




void LSPClient::sendDidOpen(const QString &uri, const QString &languageId, const QString &text) {
    QJsonObject msg{
        {"jsonrpc", "2.0"},
        {"method", "textDocument/didOpen"},
        {"params", QJsonObject{
                       {"textDocument", QJsonObject{
                                            {"uri", uri},
                                            {"languageId", languageId},
                                            {"version", 1},
                                            {"text", text}
                                        }}
                   }}
    };
    sendMessage(msg);
}

void LSPClient::sendDidChange(const QString &uri, const QString &newText) {
    QJsonObject msg{
        {"jsonrpc", "2.0"},
        {"method", "textDocument/didChange"},
        {"params", QJsonObject{
                       {"textDocument", QJsonObject{
                                            {"uri", uri},
                                            {"version", 2}
                                        }},
                       {"contentChanges", QJsonArray{
                                              QJsonObject{{"text", newText}}
                                          }}
                   }}
    };
    sendMessage(msg);
}

void LSPClient::sendCompletionRequest(const QString &uri, int line, int character) {
    // qDebug() << "send completions request methods invoke";
    // qDebug() << "uri is"<<uri;
    QJsonObject message{
        {"jsonrpc", "2.0"},
        {"id", int(nextId++)},
        {"method", "textDocument/completion"},
        {"params", QJsonObject{
                       {"textDocument", QJsonObject{{"uri", uri}}},
                       {"position", QJsonObject{{"line", line}, {"character", character}}}
                   }}
    };
    // Before sending request
    pendingRequests[message["id"].toInt()] = "completion";
    sendMessage(message);
}

void LSPClient::requestDocumentSymbols(const QString &uri) {
    //qDebug()<< "requestDocument Symbol called and uri is"<<uri ;
    QJsonObject params;
    params["textDocument"] = QJsonObject{{"uri", uri}};

    QJsonObject request;
    request["jsonrpc"] = "2.0";
    request["id"] = ++nextRequestId;
    request["method"] = "textDocument/documentSymbol";
    request["params"] = params;

    // sendMessage(request);
    pendingRequests[request["id"].toInt()] = "symbols";
     sendMessage(request);
}

void LSPClient::renameSymbol(const QString &uri, int line, int character, const QString &newName) {
    QJsonObject params{
        {"textDocument", QJsonObject{{"uri", QUrl::fromLocalFile(uri).toString()}}},
        {"position", QJsonObject{{"line", line}, {"character", character}}},
        {"newName", newName}
    };

    int id = ++nextRequestId;

    QJsonObject request{
        {"jsonrpc", "2.0"},
        {"id", id},
        {"method", "textDocument/rename"},
        {"params", params}
    };

    // ✅ Track the rename request type
    pendingRequests[request["id"].toInt()] = "rename";

    sendMessage(request);
}

void LSPClient::handleRenameResult(const QJsonObject &result) {
    //qDebug() << "Rename result:" << QJsonDocument(result).toJson(QJsonDocument::Indented);
    if (!result.contains("changes")) {
        qWarning() << "Rename result has no 'changes' field!";
        return;
    }
    QJsonObject changes = result["changes"].toObject();
    emit renameEditsReady(changes);
}

