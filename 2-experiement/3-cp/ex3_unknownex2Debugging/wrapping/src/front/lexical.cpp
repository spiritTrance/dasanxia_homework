#include "front/lexical.h"
#include "front/auxiliary_function.h"

#include<map>
#include<cassert>
#include<string>

#define TODO assert(0 && "todo")

// #define DEBUG_DFA
// #define DEBUG_SCANNER

std::string frontend::toString(State s) {
    switch (s) {
    case State::Empty: return "Empty";
    case State::Ident: return "Ident";
    case State::IntLiteral: return "IntLiteral";
    case State::FloatLiteral: return "FloatLiteral";
    case State::op: return "op";
    default:
        assert(0 && "invalid State");
    }
    return "";
}

std::set<std::string> frontend::keywords= {
    "const", "int", "float", "if", "else", "while", "continue", "break", "return", "void"
};

frontend::DFA::DFA(): cur_state(frontend::State::Empty), cur_str() {}

frontend::DFA::~DFA() {}

// flush the stored string to token
bool frontend::DFA::flush(frontend::Token& tk){
    if (this->cur_str.size()){
        tk.value = this->cur_str;
        this->cur_str = "";
        tk.type = frontend::stringToTokenType(tk.value);
        return true;
    }
    return false;
}

bool frontend::DFA::emptyStateProcess(char input, Token &buf){
    if (frontend::isOperator(input)){      // 2 op     // hard to process, so pay much attention!!!
        this->cur_str += input;
        this->cur_state = State::op;
        return false;
    }
    else if (frontend::isVarNameInit(input)){  // 2 identity
        this->cur_str += input;
        this->cur_state = State::Ident;
        return false;
    }
    else if (input == '.'){     // 2 FloatLiteral
        this->cur_str += input;
        this->cur_state = State::FloatLiteral;
        return false;
    }
    else if (frontend::isDecimalDigit(input)){ // 2 IntLiteral
        this->cur_str += input;
        this->cur_state = State::IntLiteral;
        return false;
    }
    else{
        return flush(buf);
    }
}
bool frontend::DFA::intLiteralStateProcess(char input, Token &buf){
    // especially pay attention to octal, hexadecimal!!!
    if (frontend::isProbableDigit(this->cur_str, input)){
        this->cur_str += input;
        return false;
    }
    else if (input == '.'){
        this->cur_str += input;
        this->cur_state = State::FloatLiteral;
        return false;
    }
    else if (frontend::isOperator(input)){
        bool flag = flush(buf);
        this->cur_str += input;
        this->cur_state = State::op;
        return flag;
    }
    else if (frontend::isVarNameInit(input)){
        bool flag = flush(buf);
        this->cur_str += input;
        this->cur_state = State::Ident;
        return flag;
    }
    else{
        this->cur_state = State::Empty;
        return flush(buf);
    }
}
bool frontend::DFA::floatLiteralStateProcess(char input, Token &buf){
    if (frontend::isDecimalDigit(input)){
        this->cur_str += input;
        return false;
    }
    else if (frontend::isOperator(input)){
        bool flag = flush(buf);
        this->cur_str += input;
        this->cur_state = State::op;
        return flag;
    }
    else if (frontend::isVarNameInit(input)){
        bool flag = flush(buf);
        this->cur_str += input;
        this->cur_state = State::Ident;
        return flag;
    }
    else{
        this->cur_state = State::Empty;
        return flush(buf);
    }
}
bool frontend::DFA::identityStateProcess(char input, Token &buf){
    if (frontend::isInVarCharset(input)){
        this->cur_str += input;
        return false;
    }
    else if (frontend::isOperator(input)){
        bool flag = flush(buf);
        this->cur_str += input;
        this->cur_state = State::op;
        return flag;
    }
    else{
        this->cur_state = State::Empty;
        return flush(buf);
    }
}
bool frontend::DFA::operatorStateProcess(char input, Token &buf){       // pay attention to char < > = ! & | for they have more cases, the key is to judge when to flush DFA!!!
    if (frontend::isOperator(input)){
        if (this->cur_str.size() == 1){         // length == 1 and special char probe
            bool isNeedFlush = true;
            switch(this->cur_str[0]){
                case '<':
                case '>':
                case '=':
                case '!':
                    isNeedFlush = input == '=' ? false : true;
                    break;
                case '&':
                    isNeedFlush = input == '&' ? false : true;
                    break;
                case '|':
                    isNeedFlush = input == '|' ? false : true;
                    break;
                default:
                    isNeedFlush = true;
                    break;
            }
            if (isNeedFlush){
                bool flag = flush(buf);
                this->cur_str += input;
                return flag;
            } else {
                this->cur_str += input;
                return false;
            }
        } else {                                    // length > 1 and pop out
            bool flag = flush(buf);
            this->cur_str += input;
            return flag;
        }
    } else if (frontend::isDecimalDigit(input)){
        bool flag = flush(buf);
        this->cur_str += input;
        this->cur_state = State::IntLiteral;
        return flag;
    } else if (input == '.'){
        bool flag = flush(buf);
        this->cur_str += input;
        this->cur_state = State::FloatLiteral;
        return flag;
    } else if (frontend::isVarNameInit(input)){
        bool flag = flush(buf);
        this->cur_str += input;
        this->cur_state = State::Ident;
        return flag;
    }
    else{
        bool flag = flush(buf);
        this->cur_state = State::Empty;
        return flag;
    }
}

bool frontend::DFA::next(char input, Token& buf) {
#ifdef DEBUG_DFA
#include<iostream>
    std::cout << "in state [" << toString(cur_state) << "], input = \'" << input << "\', str = $" << cur_str << "$" << std::endl;
#endif
    // TODO;
    bool ret = false;
    switch (this->cur_state)
    {
        case frontend::State::Empty:
            ret = frontend::DFA::emptyStateProcess(input, buf);
            break;
        case frontend::State::IntLiteral:
            ret = frontend::DFA::intLiteralStateProcess(input, buf);
            break;
        case frontend::State::FloatLiteral:
            ret = frontend::DFA::floatLiteralStateProcess(input, buf);
            break;
        case frontend::State::Ident:
            ret = frontend::DFA::identityStateProcess(input, buf);
            break;
        case frontend::State::op:
            ret = frontend::DFA::operatorStateProcess(input, buf);
            break;
        default:
            throw "In lexical analysis phase: unknown DFA state!!!";
    }
#ifdef DEBUG_DFA
    std::cout << "next state is [" << toString(cur_state) << "], next str = $" << cur_str << "$ , ret = " << ret << std::endl;
#endif
    return ret;
}

void frontend::DFA::reset() {
    cur_state = State::Empty;
    cur_str = "";
}

frontend::Scanner::Scanner(std::string filename): fin(filename) {
    if(!fin.is_open()) {
        assert(0 && "in Scanner constructor, input file cannot open");
    }
}

std::string frontend::Scanner::removeComments(std::ifstream& fin){
    const int IS_ROW_COMMENT = 2;      // note // cond
    const int IS_SEG_COMMENT = 1;      // note /* com */ cond
    const int IS_NOT_COMMENT = 0;
    unsigned int state = 0;
    std::string ans = "", tmp = "";
    while (getline(fin, tmp))
    {
        for (size_t i = 0; i < tmp.size();i++){
            std::string buf = tmp.substr(i, 2);
            // details: scan to the first char, so move the ptr to next and rely the for to shift ptr
            if (buf == "//"){
                state = state == IS_NOT_COMMENT ? IS_ROW_COMMENT : state;   //  case`/* // */`
                i++;
                continue;
            }
            else if (buf == "/*"){
                state = state == IS_NOT_COMMENT ? IS_SEG_COMMENT : state;   //  case`// /*`; ``// */``
                i++;
                continue;
            }
            else if (buf == "*/"){
                state = state == IS_SEG_COMMENT ? IS_NOT_COMMENT : state;   //  case`// */`
                i++;
                continue;
            }
            if (state == IS_NOT_COMMENT){
                ans += tmp[i];
            }
        }
        ans += " ";
        state = state == IS_SEG_COMMENT ? IS_SEG_COMMENT : IS_NOT_COMMENT;
    }
    return ans;
}


std::vector<frontend::Token> frontend::Scanner::run() {
    // TODO;
    std::vector<Token> ret;
    frontend::Token tk;
    frontend::DFA dfa;
    std::string s = frontend::Scanner::removeComments(this->fin);    // delete comments
    for(auto c: s) {
        if(dfa.next(c, tk)){
            ret.push_back(tk);
            #ifdef DEBUG_SCANNER
            #include<iostream>
                    std::cout << "Generate Token: " << toString(tk.type) <<" "<<tk.value<< std::endl;
            #endif
        }
    }
#ifdef DEBUG_SCANNER
#include<iostream>
    std::cout << "token: " << toString(tk.type) << "\t" << tk.value << std::endl;
#endif
            return ret;
}


frontend::Scanner::~Scanner() {
    fin.close();
}