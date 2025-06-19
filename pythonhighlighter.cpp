#include "pythonhighlighter.h"

PythonHighlighter::PythonHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    // Keywords
    QStringList keywords = {
        "and", "as", "assert", "break", "class", "continue", "def", "del",
        "elif", "else", "except", "False", "finally", "for", "from", "global",
        "if", "import", "in", "is", "lambda", "None", "nonlocal", "not", "or",
        "pass", "raise", "return", "True", "try", "while", "with", "yield"
    };

    keywordFormat.setForeground(QColor(198, 120, 221));
    keywordFormat.setFontWeight(QFont::Bold);
    for (const QString &keyword : keywords) {
        rule.pattern = QRegularExpression("\\b" + keyword + "\\b");
        rule.format = keywordFormat;
        rules.append(rule);
    }

    // Class names
    classFormat.setForeground(QColor(229, 192, 123));
    classFormat.setFontItalic(true);
    rule.pattern = QRegularExpression(R"(\bclass\s+([A-Za-z_]\w*)\b)");
    rule.format = classFormat;
    rules.append(rule);

    // Function names
    functionFormat.setForeground(QColor(97, 175, 239));
    rule.pattern = QRegularExpression(R"(\bdef\s+([A-Za-z_]\w*)\b)");
    rule.format = functionFormat;
    rules.append(rule);

    // Strings
    stringFormat.setForeground(QColor(152, 195, 121));
    rule.pattern = QRegularExpression(R"(".*?"|'.*?')");
    rule.format = stringFormat;
    rules.append(rule);

    // Numbers
    numberFormat.setForeground(QColor(209, 154, 102));
    rule.pattern = QRegularExpression(R"(\b[0-9]+(\.[0-9]+)?\b)");
    rule.format = numberFormat;
    rules.append(rule);

    // Single-line comments
    singleLineCommentFormat.setForeground(QColor(0, 128, 128));
    rule.pattern = QRegularExpression(R"(#.*$)");
    rule.format = singleLineCommentFormat;
    rules.append(rule);

    // Decorators
    decoratorFormat.setForeground(QColor(224, 108, 117));
    decoratorFormat.setFontItalic(true);
    rule.pattern = QRegularExpression(R"(@[A-Za-z_]\w*)");
    rule.format = decoratorFormat;
    rules.append(rule);

    // Operators and punctuation
    operatorFormat.setForeground(QColor(255, 255, 255));
    rule.pattern = QRegularExpression(R"([+\-*/%=&|^~<>!]=?|[\[\]{}(),.:])");
    rule.format = operatorFormat;
    rules.append(rule);

    // Multi-line strings
    multiLineStringFormat.setForeground(QColor(152, 195, 121));
    commentStartExpression = QRegularExpression(R"("""|''')");
    commentEndExpression = QRegularExpression(R"("""|''')");
}

void PythonHighlighter::highlightBlock(const QString &text)
{
    for (const HighlightingRule &rule : rules) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }

    // Multi-line strings
    setCurrentBlockState(0);
    int startIndex = 0;

    if (previousBlockState() != 1)
        startIndex = text.indexOf(commentStartExpression);

    while (startIndex >= 0) {
        QRegularExpressionMatch endMatch;
        int endIndex = text.indexOf(commentEndExpression, startIndex + 3, &endMatch);
        int commentLength = (endIndex == -1)
                                ? text.length() - startIndex
                                : endIndex - startIndex + endMatch.capturedLength();
        setFormat(startIndex, commentLength, multiLineStringFormat);
        setCurrentBlockState(1);
        startIndex = text.indexOf(commentStartExpression, startIndex + commentLength);
    }
}

