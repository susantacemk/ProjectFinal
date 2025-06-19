#ifndef PYTHONHIGHLIGHTER_H
#define PYTHONHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>

class PythonHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    PythonHighlighter(QTextDocument *parent = nullptr);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightingRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };

    QVector<HighlightingRule> rules;

    QRegularExpression commentStartExpression;
    QRegularExpression commentEndExpression;

    QTextCharFormat keywordFormat;
    QTextCharFormat classFormat;
    QTextCharFormat functionFormat;
    QTextCharFormat stringFormat;
    QTextCharFormat numberFormat;
    QTextCharFormat singleLineCommentFormat;
    QTextCharFormat decoratorFormat;
    QTextCharFormat operatorFormat;
    QTextCharFormat multiLineStringFormat;
};

#endif // PYTHONHIGHLIGHTER_H

