#include"front/token.h"
#include"front/auxiliary_function.h"

#include<cassert>

std::string frontend::toString(frontend::TokenType type){
    switch (type) {
    case TokenType::IDENFR: return "IDENFR";
    case TokenType::INTLTR: return "INTLTR";
    case TokenType::FLOATLTR: return "FLOATLTR";
    case TokenType::CONSTTK: return "CONSTTK";
    case TokenType::VOIDTK: return "VOIDTK";
    case TokenType::INTTK: return "INTTK";
    case TokenType::FLOATTK: return "FLOATTK";
    case TokenType::IFTK: return "IFTK";
    case TokenType::ELSETK: return "ELSETK";
    case TokenType::WHILETK: return "WHILETK";
    case TokenType::CONTINUETK: return "CONTINUETK";
    case TokenType::BREAKTK: return "BREAKTK";
    case TokenType::RETURNTK: return "RETURNTK";
    case TokenType::PLUS: return "PLUS";
    case TokenType::MINU: return "MINU";
    case TokenType::MULT: return "MULT";
    case TokenType::DIV: return "DIV";
    case TokenType::MOD: return "MOD";
    case TokenType::LSS: return "LSS";
    case TokenType::GTR: return "GTR";
    case TokenType::COLON: return "COLON";
    case TokenType::ASSIGN: return "ASSIGN";
    case TokenType::SEMICN: return "SEMICN";
    case TokenType::COMMA: return "COMMA";
    case TokenType::LPARENT: return "LPARENT";
    case TokenType::RPARENT: return "RPARENT";
    case TokenType::LBRACK: return "LBRACK";
    case TokenType::RBRACK: return "RBRACK";
    case TokenType::LBRACE: return "LBRACE";
    case TokenType::RBRACE: return "RBRACE";
    case TokenType::NOT: return "NOT";
    case TokenType::LEQ: return "LEQ";
    case TokenType::GEQ: return "GEQ";
    case TokenType::EQL: return "EQL";
    case TokenType::NEQ: return "NEQ";
    case TokenType::AND: return "AND";
    case TokenType::OR: return "OR";
    default:
        assert(0 && "invalid token type");
        break;
    }
    return "";
}

frontend::TokenType frontend::stringToTokenType(std::string s){
    assert(s.size() > 0 && "Zero Length string in function stringToTokenType!!!");
    if (s.size() != 1){  // length > 1
        if (s == "const")
            return frontend::TokenType::CONSTTK;
        else if (s == "void")
            return frontend::TokenType::VOIDTK;
        else if (s == "int")
            return frontend::TokenType::INTTK;
        else if (s == "float")
            return frontend::TokenType::FLOATTK;
        else if (s == "if")
            return frontend::TokenType::IFTK;
        else if (s == "else")
            return frontend::TokenType::ELSETK;
        else if (s == "while")
            return frontend::TokenType::WHILETK;
        else if (s == "continue")
            return frontend::TokenType::CONTINUETK;
        else if (s == "break")
            return frontend::TokenType::BREAKTK;
        else if (s == "return")
            return frontend::TokenType::RETURNTK;
        else if (s == "<=")
            return frontend::TokenType::LEQ;
        else if (s == ">=")
            return frontend::TokenType::GEQ;
        else if (s == "==")
            return frontend::TokenType::EQL;
        else if (s == "!=")
            return frontend::TokenType::NEQ;
        else if (s == "&&")
            return frontend::TokenType::AND;
        else if (s == "||")
            return frontend::TokenType::OR;
        else{
            if (frontend::isVarNameInit(s[0]))
                return frontend::TokenType::IDENFR;
            else{       // judge the int literal and float literal. note that the existence of hexadecimal and octal
                bool unknownCharFlag = false, digitFlag = false;
                int pointCount = 0;
                for (size_t i = 0; i < s.length();i++)
                {
                    char ch = s[i];
                    if (i <= 1 && s.length() >=2 && (s.substr(0,2) == "0b" || s.substr(0,2) == "0x"))
                        continue;           // judge the hexadecimal and binary
                    if (frontend::isProbableDigit(s, ch)){
                        digitFlag = true;
                    }
                    else if (ch == '.'){
                        pointCount++;
                        if (pointCount >= 2){
                            throw "In Phase lexical analysis: unknown string!!";
                            break;
                        }
                    }
                    else{
                        throw "In Phase lexical analysis: unknown string!!";
                        break;
                    }
                }
                if (pointCount)
                    return frontend::TokenType::FLOATLTR;
                else
                    return frontend::TokenType::INTLTR;
            }
        }
    }
    else{           // length = 1
        char ch = s[0];
        if (frontend::isDecimalDigit(ch))
            return frontend::TokenType::INTLTR;
        else if (frontend::isInVarCharset(ch))
            return frontend::TokenType::IDENFR;
        switch (ch)
        {
            case '+': return frontend::TokenType::PLUS;
            case '-': return frontend::TokenType::MINU;
            case '*': return frontend::TokenType::MULT;
            case '/': return frontend::TokenType::DIV;
            case '%': return frontend::TokenType::MOD;
            case '<': return frontend::TokenType::LSS;
            case '>': return frontend::TokenType::GTR;
            case ':': return frontend::TokenType::COLON;
            case '=': return frontend::TokenType::ASSIGN;
            case ';': return frontend::TokenType::SEMICN;
            case ',': return frontend::TokenType::COMMA;
            case '(': return frontend::TokenType::LPARENT;
            case ')': return frontend::TokenType::RPARENT;
            case '[': return frontend::TokenType::LBRACK;
            case ']': return frontend::TokenType::RBRACK;
            case '{': return frontend::TokenType::LBRACE;
            case '}': return frontend::TokenType::RBRACE;
            case '!': return frontend::TokenType::NOT;
            default:
                throw "In Phase lexical analysis: unknown string!!";
        }
    }
}