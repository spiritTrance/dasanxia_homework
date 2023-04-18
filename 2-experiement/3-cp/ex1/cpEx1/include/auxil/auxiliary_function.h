#ifndef AUXILIARY_FUNCTION
#define AUXILIARY_FUNCTION

#include "front/token.h"

namespace auxil{
    // Lexical Phase
    // - char type judge
    bool inline isOperator(char ch);
    bool inline isDigit(char ch);
    bool inline isUppercase(char ch);
    bool inline isLowerCase(char ch);
    bool inline isVarNameInit(char ch);
    bool inline isInVarCharset(char ch);
    bool inline isInLiteralCharset(char ch);
    bool inline isUselessChar(char ch);
    bool inline isInGrammarCharset(char ch);
}
#endif