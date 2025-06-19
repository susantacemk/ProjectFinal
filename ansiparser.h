#ifndef ANSIPARSER_H
#define ANSIPARSER_H

#include <QObject>
#include <QTextCharFormat>
#include <QVector>
#include<QColor>

class AnsiParser : public QObject {
    Q_OBJECT
public:
    struct AnsiState {
        QTextCharFormat format;
        int cursorX = 0;
        int cursorY = 0;
        bool bold = false;
        bool underline = false;
        bool inverse = false;
        QColor foreground;
        QColor background;
    };

    explicit AnsiParser(QObject *parent = nullptr);
    void parse(const QByteArray &data, QTextCursor &cursor);

signals:
    void titleChanged(const QString &title);

private:
    enum AnsiMode {
        Normal,
        Escape,
        CSI,  // Control Sequence Introducer
        OSC   // Operating System Command
    };

    AnsiState m_state;
    AnsiMode m_mode = Normal;
    QString m_oscBuffer;
    QVector<int> m_csiParams;
    QString m_csiIntermediate;

    void handleEscapeSequence(char ch);
    void handleCsiSequence(char ch,QTextCursor &cursor);
    void handleOscSequence(char ch);
    void applyGraphicRendition();
    void setCursorPosition(QTextCursor &cursor);
    void processCursorMovement(QTextCursor &cursor, int param, int defaultVal);
    void processErase(QTextCursor &cursor, int param);
    QColor ansiColor(int index);
};
#endif // ANSIPARSER_H
