#ifndef CPPHIGHLIGHTER_H
#define CPPHIGHLIGHTER_H


#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <QTextDocument>

class CppHighlighter : public QSyntaxHighlighter {
    Q_OBJECT

public:
    CppHighlighter(QTextDocument *parent = nullptr);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightingRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };

    QVector<HighlightingRule> rules;

    QTextCharFormat keywordFormat;
    QTextCharFormat classFormat;
    QTextCharFormat functionFormat;
    QTextCharFormat quotationFormat;
    QTextCharFormat singleLineCommentFormat;
    QTextCharFormat multiLineCommentFormat;
    QTextCharFormat preprocessorFormat;
    QTextCharFormat numberFormat;
    QTextCharFormat operatorFormat;

    QRegularExpression commentStartExpression;
    QRegularExpression commentEndExpression;
};


#endif // CPPHIGHLIGHTER_H
