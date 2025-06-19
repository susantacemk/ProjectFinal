#include "chighlighter.h"

CHighlighter::CHighlighter(QTextDocument *parent) : QSyntaxHighlighter(parent) {
    HighlightingRule rule;

    // Keywords
    keywordFormat.setForeground(QColor(18, 230, 134));
    keywordFormat.setFontWeight(QFont::Bold);
    QStringList keywords = {
        "auto", "break", "case", "char", "const", "continue", "default", "do", "double",
        "else", "enum", "extern", "float", "for", "goto", "if", "inline", "int", "long",
        "register", "restrict", "return", "short", "signed", "sizeof", "static", "struct",
        "switch", "typedef", "union", "unsigned", "void", "volatile", "while", "_Alignas",
        "_Alignof", "_Atomic", "_Bool", "_Complex", "_Generic", "_Imaginary", "_Noreturn",
        "_Static_assert", "_Thread_local"
    };

    for (const QString &kw : keywords) {
        rule.pattern = QRegularExpression("\\b" + kw + "\\b");
        rule.format = keywordFormat;
        rules.append(rule);
    }

    // Strings
    stringFormat.setForeground(QColor(219, 146, 116)); // light orange
    stringFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression("\".*?\"");
    rule.format = stringFormat;
    rules.append(rule);

    // Character literals
    charFormat.setForeground(QColor(250, 144, 100)); // darker orange
    rule.pattern = QRegularExpression("'.?'");
    rule.format = charFormat;
    rules.append(rule);

    // Numbers
    numberFormat.setForeground(QColor(252, 73, 3));
    numberFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression("\\b\\d+(\\.\\d+)?\\b");
    rule.format = numberFormat;
    rules.append(rule);

    // Preprocessor
    preprocessorFormat.setForeground(QColor(212, 157, 70));
    preprocessorFormat.setFontWeight(QFont::Bold);
    preprocessorFormat.setFontItalic(true);
    rule.pattern = QRegularExpression("^#.*");
    rule.format = preprocessorFormat;
    rules.append(rule);

    // symbol format
    QTextCharFormat symbolFormat;
    symbolFormat.setForeground(QColor(252, 3, 177));  // light gray for clarity
    symbolFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression(R"([()\[\]{};,:.])");
    rule.format = symbolFormat;
    rules.append(rule);

    // function handeling

    functionFormat.setForeground(QColor(247, 235, 5));
    functionFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression(R"(\b([A-Za-z_]\w*)\s*(?=\())");
    rule.format = functionFormat;
    rules.append(rule);

    // for handelling all input
    // Escape Sequences
    QTextCharFormat escapeFormat;
    escapeFormat.setForeground(QColor(198, 120, 221));  // Purple
    HighlightingRule escRule;
    escRule.pattern = QRegularExpression(R"(\\[abfnrtv\\'"]|\\x[0-9A-Fa-f]{2}|\\[0-7]{1,3})");
    escRule.format = escapeFormat;
    rules.append(escRule);

    // Format Specifiers
    QTextCharFormat formatSpec;
    formatSpec.setForeground(QColor(86, 182, 194)); // Teal
    HighlightingRule fmtRule;
    fmtRule.pattern = QRegularExpression(R"(%[-+0 #]*\d*(\.\d+)?[hlL]?[cdieEfgGosuxXpn%])");
    fmtRule.format = formatSpec;
    rules.append(fmtRule);

    // Single-line comments
    commentFormat.setForeground(Qt::darkGreen);
    commentFormat.setFontWeight(QFont::Bold);
    commentFormat.setFontItalic(true);
    rule.pattern = QRegularExpression("//[^\n]*");
    rule.format = commentFormat;
    rules.append(rule);

    // Multi-line comments
    commentFormat.setForeground(Qt::darkGreen);
    commentFormat.setFontWeight(QFont::Bold);
    commentFormat.setFontItalic(true);
    multiLineCommentStart = QRegularExpression("/\\*");
    multiLineCommentEnd = QRegularExpression("\\*/");



}

void CHighlighter::highlightBlock(const QString &text) {
    for (const HighlightingRule &rule : rules) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }

    // Handle multi-line comments
    setCurrentBlockState(0);
    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = text.indexOf(multiLineCommentStart);

    while (startIndex >= 0) {
        QRegularExpressionMatch endMatch;
        int endIndex = text.indexOf(multiLineCommentEnd, startIndex, &endMatch);
        int commentLength;

        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex + endMatch.capturedLength();
        }

        setFormat(startIndex, commentLength, commentFormat);
        startIndex = text.indexOf(multiLineCommentStart, startIndex + commentLength);
    }
}
