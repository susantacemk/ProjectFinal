#include "cpphighlighter.h"

CppHighlighter::CppHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent) {

    HighlightingRule rule;

    // Keywords
    QStringList keywordPatterns = {
        "\\bchar\\b", "\\bclass\\b", "\\bconst\\b", "\\bdouble\\b", "\\benum\\b", "\\bexplicit\\b",
        "\\bfriend\\b", "\\binline\\b", "\\bint\\b", "\\blong\\b", "\\bnamespace\\b", "\\boperator\\b",
        "\\bprivate\\b", "\\bprotected\\b", "\\bpublic\\b", "\\bshort\\b", "\\bsignals\\b", "\\bsigned\\b",
        "\\bslots\\b", "\\bstatic\\b", "\\bstruct\\b", "\\btemplate\\b", "\\bthis\\b", "\\bthrow\\b",
        "\\btypedef\\b", "\\btypename\\b", "\\bunion\\b", "\\bunsigned\\b", "\\bvirtual\\b", "\\bvoid\\b",
        "\\bvolatile\\b", "\\bbool\\b", "\\btrue\\b", "\\bfalse\\b", "\\bnew\\b", "\\bdelete\\b", "\\busing\\b",
        "\\btry\\b", "\\bcatch\\b", "\\boverride\\b", "\\bfinal\\b", "\\bconstexpr\\b"
    };
    keywordFormat.setForeground(QColor(198, 120, 221));
    keywordFormat.setFontWeight(QFont::Bold);
    for (const QString &pattern : keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        rules.append(rule);
    }

    // Class name
    classFormat.setForeground(QColor(229, 192, 123));
    classFormat.setFontItalic(true);
    rule.pattern = QRegularExpression(R"(\b[A-Z][A-Za-z0-9_]*\b)");
    rule.format = classFormat;
    rules.append(rule);

    // String literal
    quotationFormat.setForeground(QColor(152, 195, 121));
    rule.pattern = QRegularExpression(R"(".*?")");
    rule.format = quotationFormat;
    rules.append(rule);

    // Function name
    functionFormat.setForeground(QColor(97,175,239));
    functionFormat.setFontItalic(true);
    rule.pattern = QRegularExpression(R"(\b([A-Za-z_]\w*)\s*(?=\())");
    rule.format = functionFormat;
    rules.append(rule);

    // Numbers
    numberFormat.setForeground(QColor(209 ,154 ,102));
    rule.pattern = QRegularExpression(R"(\b[0-9]+(\.[0-9]+)?\b)");
    rule.format = numberFormat;
    rules.append(rule);

    // Preprocessor
    preprocessorFormat.setForeground(QColor(86,182,194));
    rule.pattern = QRegularExpression(R"(^\s*#\s*\w+)");
    rule.format = preprocessorFormat;
    rules.append(rule);

    // Operators and punctuation
    operatorFormat.setForeground(QColor(255,255,255));
    rule.pattern = QRegularExpression(R"([{}\[\]()<>;,.+\-*/%=&|^!~?:])");
    rule.format = operatorFormat;
    rules.append(rule);


    // Preprocessor directive
    preprocessorFormat.setForeground(QColor(86,182,194));
    preprocessorFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression(R"(^\s*#\s*(define|include|ifdef|ifndef|endif|pragma|undef))");
    rule.format = preprocessorFormat;
    rules.append(rule);

    // Macro name (after #define)
    QTextCharFormat macroNameFormat;
    macroNameFormat.setForeground(QColor(224,108,117));
    rule.pattern = QRegularExpression(R"(#define\s+(\w+))");
    rule.format = macroNameFormat;
    rules.append(rule);

    // QTextCharFormat methodDeclFormat;
    // methodDeclFormat.setForeground(QColor(0,255,255));
    // methodDeclFormat.setFontItalic(true);
    // methodDeclFormat.setFontWeight(QFont::Bold);
    // rule.pattern = QRegularExpression(R"(\b(?:void|int|float|double|char|bool|auto|long|short|unsigned|signed|class|struct)\s+([A-Za-z_]\w*)\s*\()");
    // rule.format = methodDeclFormat;
    // rules.append(rule);

    // methos handelling


    functionFormat.setForeground(QColor(247, 235, 5));
    functionFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression(R"(\b([A-Za-z_]\w*)\s*(?=\())");
    rule.format = functionFormat;
    rules.append(rule);

    // template/typename keywords
    rule.pattern = QRegularExpression(R"(\b(template|typename)\b)");
    rule.format = keywordFormat;  // reuse your keyword format
    rules.append(rule);


    QTextCharFormat templateParamFormat;
    templateParamFormat.setForeground(QColor(209, 154 ,102));  // orange-ish
    rule.pattern = QRegularExpression(R"(<\s*[\w\s,<>*&]*\s*>)");
    rule.format = templateParamFormat;
    rules.append(rule);

    QTextCharFormat namespaceFormat;
    namespaceFormat.setForeground(QColor(86,182,194));  // cyan-ish
    namespaceFormat.setFontItalic(true);
    rule.pattern = QRegularExpression(R"(\b\w+::\w+\b)");
    rule.format = namespaceFormat;
    rules.append(rule);


    // Single-line comment
    singleLineCommentFormat.setForeground(QColor(0,128,128));
    rule.pattern = QRegularExpression(R"(//[^\n]*)");
    rule.format = singleLineCommentFormat;
    rules.append(rule);

    // Multi-line comment
    multiLineCommentFormat.setForeground(QColor("0,128,128"));
    commentStartExpression = QRegularExpression(R"(/\*)");
    commentEndExpression = QRegularExpression(R"(\*/)");


}

void CppHighlighter::highlightBlock(const QString &text) {
    for (const HighlightingRule &rule : rules) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }

    // Multi-line comment handling
    setCurrentBlockState(0);

    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = text.indexOf(commentStartExpression);

    while (startIndex >= 0) {
        QRegularExpressionMatch endMatch;
        int endIndex = text.indexOf(commentEndExpression, startIndex, &endMatch);
        int commentLength = (endIndex == -1)
                                ? text.length() - startIndex
                                : endIndex - startIndex + endMatch.capturedLength();

        setFormat(startIndex, commentLength, multiLineCommentFormat);
        startIndex = text.indexOf(commentStartExpression, startIndex + commentLength);
        setCurrentBlockState(1);
    }
}

