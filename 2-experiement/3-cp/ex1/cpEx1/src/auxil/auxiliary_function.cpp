#include "auxil/auxiliary_function.h"

// note that char '.' ' ' '\n' is not in this function, and ';' is in function!
bool inline auxil::isOperator(char ch){
    switch (ch)
    {
        case '+':
        case '-':
        case '*':
        case '/':
        case '%':
        case '<':
        case '>':
        case ':':
        case '=':
        case ';':
        case ',':
        case '(':
        case ')':
        case '[':
        case ']':
        case '{':
        case '}':
        case '!':
            return true;
        default:
            return false;
    }
}

bool inline auxil::isDigit(char ch){
    return ch >= '0' && ch <= '9';
}

bool inline auxil::isUppercase(char ch){
    return ch >= 'A' && ch <= 'Z';
}

bool inline auxil::isLowerCase(char ch){
    return ch >= 'a' && ch <= 'z';
}

bool inline auxil::isVarNameInit(char ch){
    return ch == '_' || auxil::isLowerCase(ch) || auxil::isUppercase(ch);
}

bool inline auxil::isInVarCharset(char ch){
    return auxil::isVarNameInit(ch) || isDigit(ch);
}

bool inline auxil::isInLiteralCharset(char ch){
    return ch == '.' || isDigit(ch);
}

bool inline auxil::isUselessChar(char ch){
    switch (ch)
    {
    case ' ':
    case '\n':
    case '\r':
        return true;
    default:
        return false;
    }
}

bool inline auxil::isInGrammarCharset(char ch){
    return auxil::isOperator(ch) || 
            auxil::isDigit(ch) || 
            auxil::isUppercase(ch) ||
            auxil::isLowerCase(ch) ||
            ch == '.' || ch == '_';
}

