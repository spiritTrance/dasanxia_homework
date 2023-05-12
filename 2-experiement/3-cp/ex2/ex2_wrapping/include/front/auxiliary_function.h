#ifndef AUXILIARY_FUNCTION_H
#define AUXILIARY_FUNCTION_H
#include <string>

namespace frontend{
    // Lexical Phase
    // - char type judge
    bool isOperator(char ch);
    bool isDecimalDigit(char ch);
    bool isOctalDigit(char ch);
    bool isBinaryDigit(char ch);
    bool isHexaDecimalDigit(char ch);
    bool isUppercase(char ch);
    bool isLowerCase(char ch);
    bool isVarNameInit(char ch);
    bool isInVarCharset(char ch);
    bool isInLiteralCharset(char ch);
    bool isUselessChar(char ch);
    bool isInGrammarCharset(char ch);
    bool isProbableDigit(std::string s, char ch);
    int evalInt(std::string s);
    float evalFloat(std::string s);
}

#endif