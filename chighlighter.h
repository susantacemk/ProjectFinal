#ifndef CHIGHLIGHTER_H
#define CHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <QTextDocument>

class CHighlighter : public QSyntaxHighlighter {
    Q_OBJECT

public:
    CHighlighter(QTextDocument *parent = nullptr);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightingRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> rules;

    QTextCharFormat keywordFormat;
    QTextCharFormat stringFormat;
    QTextCharFormat charFormat;
    QTextCharFormat commentFormat;
    QTextCharFormat numberFormat;
    QTextCharFormat preprocessorFormat;
    QTextCharFormat functionFormat;

    QRegularExpression multiLineCommentStart;
    QRegularExpression multiLineCommentEnd;
};

#endif // CHIGHLIGHTER_H

