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
        AstNode* root = new AstNode(NodeType::TERMINAL, nullptr);
        AstNode* son = Parser::parse_exp(root);
        root->children.push_back(son);
        root->value = son->value;
        return root;
    }

    // u can define member funcition of Parser here
    template<typename T>
    T parse_exp(T fa){
        T nd = new AstNode(NodeType::EXP, fa);
        T son = Parser::parse_addExp(nd);
        nd->value = son->value;
        nd->children.push_back(son);
        return nd;
    }
    template<typename T>
    T parse_addExp(T fa){
        T nd = new AstNode(NodeType::ADDEXP, fa);
        T son1 = Parser::parse_mulExp(nd);
        nd->children.push_back(son1);
        nd->value = son1->value;
        T son2 = nullptr;
        uint32_t& i = this->index;
        while (true){
            if (i >= this->token_stream.size()) break;
            bool flag = false;
            if (this->token_stream[i].type == TokenType::PLUS){
                flag = true;
                ++i;
                son2 = Parser::parse_mulExp(nd);
                nd->value += son2->value;
            }
            else if (this->token_stream[i].type == TokenType::MINU){
                flag = true;
                ++i;
                son2 = Parser::parse_mulExp(nd);
                nd->value -= son2->value;
            }
            if (!flag) break;
        }
        if (son2!=nullptr){
            nd->children.push_back(son2);
        }
        return nd;
    }
    template<typename T>
    T parse_mulExp(T fa){
        T nd = new AstNode(NodeType::MULEXP, fa);
        T son1 = Parser::parse_unaryExp(nd);
        nd->children.push_back(son1);
        nd->value = son1->value;
        T son2 = nullptr;
        uint32_t& i = this->index;
        while (true){
            if (i >= this->token_stream.size()) break;
            bool flag = false;
            if (this->token_stream[i].type == TokenType::MULT){
                ++i;
                flag = true;
                son2 = Parser::parse_unaryExp(nd);
                nd->value *= son2->value;
            }
            else if (this->token_stream[i].type == TokenType::DIV){
                ++i;
                flag = true;
                son2 = Parser::parse_unaryExp(nd);
                nd->value /= son2->value;
            }
            if (!flag) break;
        }
        if (son2!=nullptr){
                nd->children.push_back(son2);
        }
        return nd;
    }
    template<typename T>
    T parse_unaryExp(T fa){
        T nd = new AstNode(NodeType::UNARYEXP, fa);
        uint32_t &i = this->index;
        if (i >= this->token_stream.size())
                goto label1;
        if (this->token_stream[i].type == TokenType::PLUS)
        {
                i++;
                nd->children.push_back(new AstNode(NodeType::UNARYOP, nd));
                T son = parse_unaryExp(nd);
                nd->value = son->value;
                nd->children.push_back(son);
            return nd;
        }
        else if (this->token_stream[i].type == TokenType::MINU){
            i++;
            nd->children.push_back(new AstNode(NodeType::UNARYOP, nd));
            T son = parse_unaryExp(nd);
            nd->value = -son->value;
            nd->children.push_back(son);
            return nd;
        }
        label1:
            T son = Parser::parse_primaryExp(nd);
            nd->children.push_back(son);
            nd->value = son->value;
        return nd;
    }
    template<typename T>
    T parse_primaryExp(T fa){
        T nd = new AstNode(NodeType::PRIMARYEXP, fa);
        uint32_t &i = this->index;
        if (i >= this->token_stream.size())
            goto label2;
        if (this->token_stream[i].type == TokenType::LPARENT)
        {
            i++; // 左括号的右边的token
            T son = Parser::parse_exp(nd);
            while (i < this->token_stream.size() && this->token_stream[i].type != TokenType::RPARENT)
            {
                i++;
            }
            i++;
            nd->children.push_back(son);
            nd->value = son->value;
            return nd;
        }
        label2:
            T son = Parser::parse_number(nd);
            nd->children.push_back(son);
            nd->value = son->value;
        return nd;
    }

    template<typename T>
    T parse_number(T fa){
        T nd = new AstNode(NodeType::NUMBER, fa);
        std::string value = this->token_stream[this->index++].value;
        int ndVal = 0;
        uint32_t radixBase = 10;
        if (value.length() < 2){
            value = "0d" + value;
        }
        std::string ss = value.substr(0, 2);
        if (ss=="0x")
            radixBase = 16;
        else if (ss == "0b")
            radixBase = 2;
        else if (ss == "0d"){
            radixBase = 10;
        }
        else if (ss[0] == '0'){
            radixBase = 8;
            value = "0o" + value;
        }
        else{
            radixBase = 10;
            value = "0d" + value;
        }
        for (size_t i = 0; i < value.substr(2).size();i++)
        {
        #ifdef DEBUG_PARSER
                std::cout << "Targeted Val: " <<value <<" ,Cur Val: "<<ndVal << std::endl;
        #endif
                char ch = value[i + 2];
                uint32_t radixVal;
                if (ch >= 'a' && ch <= 'f')
                    radixVal = ch - 'a' + 10;
                else if (ch >= 'A' && ch <= 'F')
                    radixVal = ch - 'A' + 10;
                else
                    radixVal = ch - '0';
                ndVal = ndVal * radixBase + radixVal;
        }
        nd->value = ndVal;
        return nd;
    }

// for debug, u r not required to use this
// how to use this: in ur local enviroment, defines the macro DEBUG_PARSER and add this function in every parse fuction
void log(AstNode* node){
        #ifdef DEBUG_PARSER
                // return;
                std::string ans= "";
                for (Token tk: token_stream){
                        ans += tk.value;
                }
                if (index >= token_stream.size() && node!=nullptr){
                std::cout       << "in parse "          << toString(node->type) 
                                << "\t node_val::"      << node->value 
                                // << "\t fa_type::"      << toString(node->parent->type) 
                                << "\t cur_index::"       <<index
                                << "\t"+ans
                                <<  std::endl;
                }
                else if (node == nullptr){
                std::cout << "Nullptr" << std::endl;
                }
                else
                std::cout       << "in parse "          << toString(node->type) 
                                << "\t node_val::"      << node->value 
                                // << "\t fa_type::"      << toString(node->parent->type) 
                                << "\t cur_index::"       <<index
                                << "\t cur_token_type::" << toString(token_stream[index].type) 
                                << "\t token_val::"      << token_stream[index].value 
                                << "\t"+ans
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

