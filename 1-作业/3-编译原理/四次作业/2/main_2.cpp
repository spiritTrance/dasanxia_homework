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
    if (
        (input >= '0' && input <= '9') || 
        (input >= 'a' && input <= 'f') || 
        (input >= 'A' && input <= 'F') || 
        input == 'x' || 
        input == 'b' || 
        input == 'o'
    ){
        if (DFA::cur_state == State::op){
            buf.type = get_op_type(DFA::cur_str);
            buf.value = DFA::cur_str;
            DFA::cur_str = std::string(1, input);
            DFA::cur_state = State::IntLiteral;
            return true;
        }
        else{
            DFA::cur_str = DFA::cur_str + input;
            DFA::cur_state = State::IntLiteral;
            return false;
        }
    }
    else if (isoperator(input)){
        buf.type = get_op_type(DFA::cur_str);
        buf.value = DFA::cur_str;
        DFA::cur_str = std::string(1, input);
        DFA::cur_state = State::op;
        return buf.value.size();
    }
    else{
        buf.type = get_op_type(DFA::cur_str);
        buf.value = DFA::cur_str;
        DFA::reset();
        return buf.value.size();
    }

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
        AstNode *root = new AstNode(NodeType::TERMINAL, nullptr);
        AstNode *son = Parser::parse_exp(root);
        root->children.push_back(son);
        root->value = son->value;
        return root;
    
    }

    // u can define member funcition of Parser here
AstNode* parse_exp(AstNode* fa){
        AstNode *expNode = new AstNode(NodeType::EXP, fa);
        AstNode *son = Parser::parse_addExp(expNode);
        expNode->children.push_back(son);
        expNode->value = son->value;
        
        return expNode;
    }
    AstNode* parse_addExp(AstNode* fa){
        AstNode *addExpNode = new AstNode(NodeType::ADDEXP, fa);
        AstNode *mulExpSon_1 = Parser::parse_mulExp(addExpNode);
        addExpNode->children.push_back(mulExpSon_1);
        addExpNode->value = mulExpSon_1->value;
        AstNode *mulExpSon_2 = nullptr;
        uint32_t& i = this->index;
        // 判断是否有期望的符号
        
        if (i < this->token_stream.size()){
                if (this->token_stream[i].type == TokenType::PLUS){
                        ++i;
                        mulExpSon_2 = Parser::parse_mulExp(addExpNode);
                        addExpNode->value += mulExpSon_2->value;
                }
                else if (this->token_stream[i].type == TokenType::MINU){
                        ++i;
                        mulExpSon_2 = Parser::parse_mulExp(addExpNode);
                        addExpNode->value -= mulExpSon_2->value;
                }
        }
        if (mulExpSon_2!=nullptr){
                addExpNode->children.push_back(mulExpSon_2);
        }
        
        return addExpNode;
    }
    AstNode* parse_mulExp(AstNode* fa){
        AstNode *mulExpNode = new AstNode(NodeType::MULEXP, fa);
        AstNode *unaryExp_1 = Parser::parse_unaryExp(mulExpNode);
        mulExpNode->children.push_back(unaryExp_1);
        mulExpNode->value = unaryExp_1->value;
        AstNode *mulExpSon_2 = nullptr;
        uint32_t& i = this->index;
        // 判断是否有期望的符号
        if (i < this->token_stream.size()){
                if (this->token_stream[i].type == TokenType::MULT){
                        ++i;
                        mulExpSon_2 = Parser::parse_unaryExp(mulExpNode);
                        mulExpNode->value *= mulExpSon_2->value;
                }
                else if (this->token_stream[i].type == TokenType::DIV){
                        ++i;
                        mulExpSon_2 = Parser::parse_unaryExp(mulExpNode);
                        mulExpNode->value /= mulExpSon_2->value;
                }
        }
        if (mulExpSon_2!=nullptr){
                mulExpNode->children.push_back(mulExpSon_2);
        }
        
        return mulExpNode;
    }
    AstNode* parse_unaryExp(AstNode* fa){
        AstNode *unaryExpNode = new AstNode(NodeType::UNARYEXP, fa);
        uint32_t &i = this->index;
        // 儿子
        if (i < this->token_stream.size() && this->token_stream[i].type == TokenType::PLUS){
                i++;
                unaryExpNode->children.push_back(new AstNode(NodeType::UNARYOP, unaryExpNode));
                AstNode *son = parse_unaryExp(unaryExpNode);
                unaryExpNode->value = son->value;
                unaryExpNode->children.push_back(son);
        }
        else if (i < this->token_stream.size() && this->token_stream[i].type == TokenType::MINU){
                i++;
                unaryExpNode->children.push_back(new AstNode(NodeType::UNARYOP, unaryExpNode));
                AstNode *son = parse_unaryExp(unaryExpNode);
                unaryExpNode->value = -son->value;
                unaryExpNode->children.push_back(son);
        }
        else{
                AstNode *son = Parser::parse_primaryExp(unaryExpNode);
                unaryExpNode->children.push_back(son);
                unaryExpNode->value = son->value;
        }
        return unaryExpNode;
    }
    AstNode* parse_primaryExp(AstNode* fa){
        AstNode *primaryExpNode = new AstNode(NodeType::PRIMARYEXP, fa);
        
        uint32_t &i = this->index;
        if (i < this->token_stream.size() && this->token_stream[i].type == TokenType::LPARENT){
                i++;    // 左括号的右边的token
                AstNode *son = Parser::parse_exp(primaryExpNode);
                while (i < this->token_stream.size() && this->token_stream[i].type != TokenType::RPARENT)
                {
                        i++;
                }
                i++;
                primaryExpNode->children.push_back(son);
                primaryExpNode->value = son->value;
        }
        else{
                AstNode *son = Parser::parse_number(primaryExpNode);
                primaryExpNode->children.push_back(son);
                primaryExpNode->value = son->value;
        }
        return primaryExpNode;
    }
    AstNode* parse_number(AstNode* fa){
        AstNode *numberNode = new AstNode(NodeType::NUMBER, fa);
        std::string value = this->token_stream[this->index++].value;
        uint32_t nd_val = 0;
        uint32_t base = 10;
        if (value.length() < 2){
                value = "0d" + value;
        }
        if (value.length() >= 2){
                switch (value[1])
                {
                case 'o':
                        base = 8;
                        break;
                case 'x':
                        base = 16;
                        break;
                case 'b':
                        base = 2;
                        break;
                case 'd':
                        base = 10;
                        break;
                default:
                        value = "0d" + value;
                        base = 10;
                        break;
                }
        }
        if (value[0] == '0')
            base = 8;
        for (char ch : value.substr(2))
        {
                uint32_t radixVal = (ch>='a' && ch<='f') ? ch - 'a' + 10 :
                               (ch>='A' && ch<='F') ? ch - 'A' + 10 : ch - '0';
                nd_val = nd_val * base + radixVal;
        }
        numberNode->value = nd_val;
        return numberNode;
    }

// for debug, u r not required to use this
// how to use this: in ur local enviroment, defines the macro DEBUG_PARSER and add this function in every parse fuction
void log(AstNode* node){
        #ifdef DEBUG_PARSER
            std::cout       << " in parse "          << toString(node->type) 
                            << " node_val::"        << node->value 
                            << " cur_index::"       <<index
                            << " cur_token_type::"  << toString(token_stream[index].type) 
                            << " token_val::"       << token_stream[index].value 
                            <<  std::endl;
        #endif
    }


    
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

