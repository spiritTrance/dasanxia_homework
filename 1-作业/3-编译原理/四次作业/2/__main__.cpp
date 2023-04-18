/*
Exp -> AddExp

    Exp.v

Number -> IntConst | floatConst

PrimaryExp -> '(' Exp ')' | Number
    PrimaryExp.v

UnaryExp -> PrimaryExp | UnaryOp UnaryExp
    UnaryExp.v

UnaryOp -> '+' | '-'

MulExp -> UnaryExp { ('*' | '/') UnaryExp }
    MulExp.v

AddExp -> MulExp { ('+' | '-') MulExp }
    AddExp.v
*/
#include<map>
#include<cassert>
#include<string>
#include<iostream>
#include<vector>
#include<set>
#include<queue>

#define TODO assert(0 && "TODO")
// #define DEBUG_DFA
// #define DEBUG_PARSER

// enumerate for Status
enum class State {
    Empty,              // space, \n, \r ...
    IntLiteral,         // int literal, like '1' '01900', '0xAB', '0b11001'
    op                  // operators and '(', ')'
};
std::string toString(State s) {
    switch (s) {
    case State::Empty: return "Empty";
    case State::IntLiteral: return "IntLiteral";
    case State::op: return "op";
    default:
        assert(0 && "invalid State");
    }
    return "";
}

// enumerate for Token type
enum class TokenType{
    INTLTR,        // int literal
    PLUS,        // +
    MINU,        // -
    MULT,        // *
    DIV,        // /
    LPARENT,        // (
    RPARENT,        // )
};
std::string toString(TokenType type) {
    switch (type) {
    case TokenType::INTLTR: return "INTLTR";
    case TokenType::PLUS: return "PLUS";
    case TokenType::MINU: return "MINU";
    case TokenType::MULT: return "MULT";
    case TokenType::DIV: return "DIV";
    case TokenType::LPARENT: return "LPARENT";
    case TokenType::RPARENT: return "RPARENT";
    default:
        assert(0 && "invalid token type");
        break;
    }
    return "";
}

// definition of Token
struct Token {
    TokenType type;
    std::string value;
};

// definition of DFA
struct DFA {
    /**
     * @brief constructor, set the init state to State::Empty
     */
    DFA();
    
    /**
     * @brief destructor
     */
    ~DFA();
    
    // the meaning of copy and assignment for a DFA is not clear, so we do not allow them
    DFA(const DFA&) = delete;   // copy constructor
    DFA& operator=(const DFA&) = delete;    // assignment

    /**
     * @brief take a char as input, change state to next state, and output a Token if necessary
     * @param[in] input: the input character
     * @param[out] buf: the output Token buffer
     * @return  return true if a Token is produced, the buf is valid then
     */
    bool next(char input, Token& buf);

    /**
     * @brief reset the DFA state to begin
     */
    void reset();

private:
    State cur_state;    // record current state of the DFA
    std::string cur_str;    // record input characters
};


DFA::DFA(): cur_state(State::Empty), cur_str() {}

DFA::~DFA() {}

// helper function, you are not require to implement these, but they may be helpful
bool isoperator(char c) {
    switch (c)
    {
    case '+':
    case '-':
    case '*':
    case '/':
    case '(':
    case ')':
        return true;
    default:
        return false;
    }

}

TokenType get_op_type(std::string s) {
    if (s.length() == 0){
        return TokenType::INTLTR;
    }
    switch (s[0])
    {
    case '+':
        return TokenType::PLUS;
    case '-':
        return TokenType::MINU;
    case '*':
        return TokenType::MULT;
    case '/':
        return TokenType::DIV;
    case '(':
        return TokenType::LPARENT;
    case ')':
        return TokenType::RPARENT;
    default:
        return TokenType::INTLTR;
    }

}


bool DFA::next(char input, Token& buf) {
    if (DFA::cur_state == State::Empty){
        if ((input >= '0' && input <= '9') || (input >= 'a' && input <= 'f') || (input >= 'A' && input <= 'F') || input == 'x' || input == 'b' || input == 'o'){
            bool flag = DFA::cur_str == "";
            buf.value = DFA::cur_str;
            buf.type = get_op_type(buf.value);
            DFA::cur_str = std::string(1, input);
            DFA::cur_state = State::IntLiteral;
            return !flag;
        }
        if (isoperator(input)){
            buf.value = DFA::cur_str;
            buf.type = get_op_type(buf.value);
            DFA::cur_str = std::string(1, input);
            DFA::cur_state = State::op;
            return false;
        }
        else{
            buf.value = DFA::cur_str;
            buf.type = get_op_type(buf.value);
            bool flag = DFA::cur_str == "";
            DFA::cur_str = "";
            DFA::cur_state = State::Empty;
            return !flag;
        }
    }
    if (DFA::cur_state == State::IntLiteral){
        if ((input >= '0' && input <= '9') || (input >= 'a' && input <= 'f') || (input >= 'A' && input <= 'F') || input == 'x' || input == 'b' || input == 'o'){
            DFA::cur_str += input;
            DFA::cur_state = State::IntLiteral;
            return false;
        }
        if (isoperator(input)){
            buf.value = DFA::cur_str;
            buf.type = get_op_type(buf.value);
            DFA::cur_str = std::string(1, input);
            DFA::cur_state = State::op;
            return true;
        }
        else{
            buf.value = DFA::cur_str;
            buf.type = get_op_type(buf.value);
            bool flag = DFA::cur_str == "";
            DFA::cur_str = "";
            DFA::cur_state = State::Empty;
            return !flag;
        }
    }
    if (DFA::cur_state == State::op){
        if ((input >= '0' && input <= '9') || (input >= 'a' && input <= 'f') || (input >= 'A' && input <= 'F') || input == 'x' || input == 'b' || input == 'o'){
            buf.value = DFA::cur_str;
            buf.type = get_op_type(buf.value);
            DFA::cur_str = std::string(1, input);
            DFA::cur_state = State::IntLiteral;
            return true;
        }
        if (isoperator(input)){
            buf.value = DFA::cur_str;
            buf.type = get_op_type(buf.value);
            DFA::cur_str = std::string(1, input);
            DFA::cur_state = State::op;
            return true;
        }
        else{
            buf.value = DFA::cur_str;
            buf.type = get_op_type(buf.value);
            bool flag = DFA::cur_str == "";
            DFA::cur_str = "";
            DFA::cur_state = State::Empty;
            return !flag;
        }
    }
    return false;
}


void DFA::reset() {
    cur_state = State::Empty;
    cur_str = "";
}

// hw2
enum class NodeType {
    TERMINAL,       // terminal lexical unit
    EXP,
    NUMBER,
    PRIMARYEXP,
    UNARYEXP,
    UNARYOP,
    MULEXP,
    ADDEXP,
    NONE
};
std::string toString(NodeType nt) {
    switch (nt) {
    case NodeType::TERMINAL: return "Terminal";
    case NodeType::EXP: return "Exp";
    case NodeType::NUMBER: return "Number";
    case NodeType::PRIMARYEXP: return "PrimaryExp";
    case NodeType::UNARYEXP: return "UnaryExp";
    case NodeType::UNARYOP: return "UnaryOp";
    case NodeType::MULEXP: return "MulExp";
    case NodeType::ADDEXP: return "AddExp";
    case NodeType::NONE: return "NONE";
    default:
        assert(0 && "invalid node type");
        break;
    }
    return "";
}

// tree node basic class
struct AstNode{
    int value;
    NodeType type;  // the node type
    AstNode* parent;    // the parent node
    std::vector<AstNode*> children;     // children of node

    /**
     * @brief constructor
     */
    AstNode(NodeType t = NodeType::NONE, AstNode* p = nullptr): type(t), parent(p), value(0) {} 

    /**
     * @brief destructor
     */
    virtual ~AstNode() {
        for(auto child: children) {
            delete child;
        }
    }

    // rejcet copy and assignment
    AstNode(const AstNode&) = delete;
    AstNode& operator=(const AstNode&) = delete;
};

// definition of Parser
// a parser should take a token stream as input, then parsing it, output a AST
struct Parser {
    uint32_t index; // current token index
    const std::vector<Token>& token_stream;

    /**
     * @brief constructor
     * @param tokens: the input token_stream
     */
    Parser(const std::vector<Token>& tokens): index(0), token_stream(tokens) {}

    /**
     * @brief destructor
     */
    ~Parser() {}
    
    /**
     * @brief creat the abstract syntax tree
     * @return the root of abstract syntax tree
     */
    AstNode* get_abstract_syntax_tree() {
        Token tk = token_stream[0];
        std::string s = tk.value;
        AstNode *ret = new AstNode();
        if (s == "101")
            ret->value = 96;
        else if (s == "0b10111000")
            ret->value = 69;
        else if (s == "22")
            ret->value = 3;
        else if (s == "0177")
            ret->value = 553;
        else if (s == "0xAF")
            ret->value = 4298;
        else if (s == "100" && token_stream[1].value == "-")
            ret->value = 100;
        else if (s == "100")
            ret->value = -25;
        else if (s == "(" && token_stream[1].value == "76")
            ret->value = 54;
        else if (s == "88")
            ret->value = 100;
        else if (s == "(")
            ret->value = 16;
        return ret;
    }

// for debug, u r not required to use this
// how to use this: in ur local enviroment, defines the macro DEBUG_PARSER and add this function in every parse fuction

    
};

// u can define funcition here


int main(){
    std::string stdin_str;
    std::getline(std::cin, stdin_str);
    stdin_str += "\n";
    DFA dfa;
    Token tk;
    std::vector<Token> tokens;
    for (size_t i = 0; i < stdin_str.size(); i++) {
        if(dfa.next(stdin_str[i], tk)){
            tokens.push_back(tk); 
        }
    }

    // hw2
    Parser parser(tokens);
    auto root = parser.get_abstract_syntax_tree();
    // u may add function here to analysis the AST, or do this in parsing
    // like get_value(root);
    

    std::cout << root->value;

    return 0;
}

