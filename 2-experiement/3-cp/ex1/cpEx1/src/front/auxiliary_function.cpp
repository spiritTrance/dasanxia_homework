#include "front/auxiliary_function.h"

#include <string>
// note that char '.' ' ' '\n' is not in this function, and ';' is in function!
bool frontend::isOperator(char ch){
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
        case '&':
        case '|':
            return true;
        default:
            return false;
    }
}

bool frontend::isDecimalDigit(char ch){
    return ch >= '0' && ch <= '9';
}

bool frontend::isBinaryDigit(char ch){
    return ch == '0' || ch == '1';
}

bool frontend::isOctalDigit(char ch){
    return ch >= '0' && ch <= '7';
}

bool frontend::isHexaDecimalDigit(char ch){
    return (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f') || (ch >='A' && ch <='F');
}

bool frontend::isUppercase(char ch){
    return ch >= 'A' && ch <= 'Z';
}

bool frontend::isLowerCase(char ch){
    return ch >= 'a' && ch <= 'z';
}

bool frontend::isVarNameInit(char ch){
    return ch == '_' || frontend::isLowerCase(ch) || frontend::isUppercase(ch);
}

bool frontend::isInVarCharset(char ch){
    return frontend::isVarNameInit(ch) || isDecimalDigit(ch);
}

bool frontend::isInLiteralCharset(char ch){
    return ch == '.' || isDecimalDigit(ch);
}

bool frontend::isUselessChar(char ch){
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

bool frontend::isInGrammarCharset(char ch){
    return frontend::isOperator(ch) || 
            frontend::isDecimalDigit(ch) || 
            frontend::isUppercase(ch) ||
            frontend::isLowerCase(ch) ||
            ch == '.' || ch == '_';
}

bool frontend::isProbableDigit(std::string s, char ch){
    if (s.length() == 0){
        return frontend::isDecimalDigit(ch);
    }
    else if (s.length() == 1){
        if (ch == 'b' || ch == 'x')
            return true;
        else if (s[0] == '0')
            return frontend::isOctalDigit(ch);
        else
            return frontend::isDecimalDigit(ch);
    }
    else{
        std::string ss = s.substr(0, 2);
        if (ss == "0b"){
            return frontend::isBinaryDigit(ch);
        } else if (ss == "0x"){
            return frontend::isHexaDecimalDigit(ch);
        } else if (s[0] == '0'){
            return frontend::isOctalDigit(ch);
        } else {
            return frontend::isDecimalDigit(ch);
        }
    }
}