#ifndef LSPCLIENT_H
#define LSPCLIENT_H

#include <QObject>
#include <QProcess>
#include <QJsonObject>
#include <QJsonArray>

class LSPClient : public QObject {
    Q_OBJECT

public:
    ~LSPClient();
    explicit LSPClient(QObject *parent = nullptr);
    void start(const QString &rootPath);
    void sendDidOpen(const QString &uri, const QString &languageId, const QString &text);
    void sendDidChange(const QString &uri, const QString &newText);
    void requestDiagnostics(const QString &uri);
    void sendCompletionRequest(const QString &uri, int line, int character);
    void requestDocumentSymbols(const QString &uri);
    void renameSymbol(const QString &uri, int line, int character, const QString &newName);
    void handleRenameResult(const QJsonObject &result);

signals:
    void renameEditsReady(const QJsonObject &changes);
signals:
    void documentSymbolsReceived(const QJsonArray &symbols);
signals:
    void completionsReceived(const QStringList &suggestions);


signals:
    void diagnosticsReceived(const QJsonArray &diagnostics);

private slots:
    void readFromServer();

private:
    QProcess process;
    quint64 nextId = 1;
    QByteArray buffer;
    void sendMessage(const QJsonObject &msg);

    // symbol table
    int nextRequestId = 1;  // To uniquely ID each LSP request
    QMap<int, QString> pendingRequests;  // Maps request ID → request type
};

#endif // LSPCLIENT_H

