#include"front/syntax.h"

#include<iostream>
#include<cassert>

using frontend::Parser;

// #define DEBUG_PARSER
#define TODO assert(0 && "todo")
// 1.根据下一个 Token 类型的类型选择处理的产生式(一般只看下一个 Token 就可以选择产生式，少数情况下多个产生式的 first 集有交集时，应多向后看几个 Token)
#define CUR_TOKEN_IS(tk_type) (token_stream[index].type == TokenType::tk_type)
#define CUR_TOKEN_IS_N_STEP(tk_type, n) ((index + n < token_stream.size()) && (token_stream[index + n].type == TokenType::tk_type))
// 2.如果是非终结符，则调用其 parse 函数，并将其挂在 root 节点上
#define PARSE_TOKEN(tk_type) root->children.push_back(parseTerm(root, TokenType::tk_type))
// 3.如果是终结符，则调用 parseTerm 函数，并将其挂在 root 节点上
#define PARSE(name, type) auto name = new type(root); assert(parse##type(name)); root->children.push_back(name);

#ifdef DEBUG_PARSER
    static int DEBUG_NUM = 0;
#endif

Parser::Parser(const std::vector<frontend::Token>& tokens): index(0), token_stream(tokens) {}

Parser::~Parser() {}

frontend::CompUnit* Parser::get_abstract_syntax_tree(){
    CompUnit *root = new CompUnit();
    Parser::parseCompUnit(root);
    return root;
}

frontend::Term *Parser::parseTerm(AstNode *parent, TokenType expected){

    if (index >= token_stream.size()){
        assert(0 && "In syntax.cpp: Exceed index!");
    }
    if (token_stream[index].type != expected){
        assert(0 && "In syntax.cpp: Unexpected TokenType!");
    }
    Token tk = token_stream[index++];
    return new frontend::Term(tk, parent);
}

// Pay attention to terminal: Ident、IntConst、floatConst
// pay attention that multiple choice!
// CompUnit -> (Decl | FuncDef) [CompUnit]
bool Parser::parseCompUnit(CompUnit* root){
    log(root);
    if (CUR_TOKEN_IS(CONSTTK)){
        PARSE(decl, Decl);
    }
    else if (CUR_TOKEN_IS_N_STEP(LPARENT, 2)){
        PARSE(funcDef, FuncDef);
    }
    else{
        PARSE(decl, Decl);
    }
    if (CUR_TOKEN_IS(CONSTTK) || CUR_TOKEN_IS(INTTK) || CUR_TOKEN_IS(FLOATTK)){
        PARSE(compUnit, CompUnit);
    }
    log(root, 0);
    return true;
}

// Decl -> ConstDecl | VarDecl
bool Parser::parseDecl(Decl* root){
    log(root);
    if (CUR_TOKEN_IS(CONSTTK)){
        PARSE(constDecl, ConstDecl);
    }
    else{
        PARSE(varDecl, VarDecl);
    }
    log(root, 0);
    return true;
}

// FuncDef -> FuncType Ident '(' [FuncFParams] ')' Block
bool Parser::parseFuncDef(FuncDef* root){
    log(root);
    PARSE(functype, FuncType);
    PARSE_TOKEN(IDENFR);
    PARSE_TOKEN(LPARENT);
    // no [FuncFParams], FuncType Ident '(' ')' Block
    if(CUR_TOKEN_IS(RPARENT)) {
        PARSE_TOKEN(RPARENT);
    }
    // FuncType Ident '(' FuncFParams ')' Block
    else {
        PARSE(node, FuncFParams);
        PARSE_TOKEN(RPARENT);
    }
    PARSE(block, Block);
    log(root, 0);
    return true;
}

// ConstDecl -> 'const' BType ConstDef { ',' ConstDef } ';'
bool Parser::parseConstDecl(ConstDecl* root){
    log(root);
    PARSE_TOKEN(CONSTTK);
    PARSE(bType, BType);
    PARSE(constDef, ConstDef);
    while(CUR_TOKEN_IS(COMMA)){
        PARSE_TOKEN(COMMA);
        PARSE(constDef, ConstDef);
    }
    PARSE_TOKEN(SEMICN);
    log(root, 0);
    return true;
}

// VarDecl -> BType VarDef { ',' VarDef } ';'
bool Parser::parseVarDecl(VarDecl* root){
    log(root);
    PARSE(bType, BType);
    PARSE(varDef, VarDef);
    while(CUR_TOKEN_IS(COMMA)){
        PARSE_TOKEN(COMMA);
        PARSE(varDef, VarDef);
    }
    PARSE_TOKEN(SEMICN);
    log(root, 0);
    return true;
}

// BType -> 'int' | 'float'
bool Parser::parseBType(BType* root){
    log(root);
    if (CUR_TOKEN_IS(INTTK)){
        PARSE_TOKEN(INTTK);
    }
    else if (CUR_TOKEN_IS(FLOATTK)){
        PARSE_TOKEN(FLOATTK);
    }
    else{
        assert(0 && "Syntax.cpp: in function parseBType: undesired token.");
        return false;
    }
    log(root, 0);
    return true;
}

// ConstDef -> Ident { '[' ConstExp ']' } '=' ConstInitVal
bool Parser::parseConstDef(ConstDef* root){
    log(root);
    PARSE_TOKEN(IDENFR);
    while(CUR_TOKEN_IS(LBRACK)){
        PARSE_TOKEN(LBRACK);
        PARSE(constExp, ConstExp);
        PARSE_TOKEN(RBRACK);
    }
    PARSE_TOKEN(ASSIGN);
    PARSE(constInitVal, ConstInitVal);
    log(root, 0);
    return true;
}

// ConstInitVal -> ConstExp | '{' [ ConstInitVal { ',' ConstInitVal } ] '}'
bool Parser::parseConstInitVal(ConstInitVal* root){
    log(root);
    if (CUR_TOKEN_IS(LBRACE)){
        PARSE_TOKEN(LBRACE);
        if (!CUR_TOKEN_IS(RBRACE)){
            PARSE(constInitVal, ConstInitVal);
            while(CUR_TOKEN_IS(COMMA)){
                PARSE_TOKEN(COMMA);
                PARSE(constInitVal, ConstInitVal);
            }
        }
        PARSE_TOKEN(RBRACE);
    }
    else{
        PARSE(constExp, ConstExp);
    }
    log(root, 0);
    return true;
}

// ConstExp -> AddExp
bool Parser::parseConstExp(ConstExp* root){
    log(root);
    PARSE(addExp, AddExp);
    log(root, 0);
    return true;
}

// VarDef -> Ident { '[' ConstExp ']' } [ '=' InitVal ]
bool Parser::parseVarDef(VarDef* root){
    log(root);
    PARSE_TOKEN(IDENFR);
    while (CUR_TOKEN_IS(LBRACK))
    {
        PARSE_TOKEN(LBRACK);
        PARSE(constExp, ConstExp);
        PARSE_TOKEN(RBRACK);
    }
    if (CUR_TOKEN_IS(ASSIGN)){
        PARSE_TOKEN(ASSIGN);
        PARSE(initVal, InitVal);
    }
    log(root, 0);
    return true;
}

// InitVal -> Exp | '{' [ InitVal { ',' InitVal } ] '}'
bool Parser::parseInitVal(InitVal* root){
    log(root);
    if (CUR_TOKEN_IS(LBRACE)){
        PARSE_TOKEN(LBRACE);
        if (!CUR_TOKEN_IS(RBRACE)){
            PARSE(initVal, InitVal);
            while(CUR_TOKEN_IS(COMMA)){
                PARSE_TOKEN(COMMA);
                PARSE(initVal2, InitVal);
            }
        }
        PARSE_TOKEN(RBRACE);
    }
    else{
        PARSE(exp, Exp);
    }
    log(root, 0);
    return true;
}

// Exp -> AddExp
bool Parser::parseExp(Exp* root){
    log(root);
    PARSE(addExp, AddExp)
    log(root, 0);
    return true;
}

// FuncType -> 'void' | 'int' | 'float'
bool Parser::parseFuncType(FuncType* root){
    log(root);
    if (CUR_TOKEN_IS(VOIDTK)){
        PARSE_TOKEN(VOIDTK);
    }
    else if (CUR_TOKEN_IS(INTTK)){
        PARSE_TOKEN(INTTK);
    }
    else if (CUR_TOKEN_IS(FLOATTK)){
        PARSE_TOKEN(FLOATTK);
    }
    else{
        assert(0 && "In syntax.cpp: function parseFuncType: undesired token");
        return false;
    }
    log(root, 0);
    return true;
}

// FuncFParam -> BType Ident ['[' ']' { '[' Exp ']' }]
bool Parser::parseFuncFParam(FuncFParam* root){
    log(root);
    PARSE(btype, BType);
    PARSE_TOKEN(IDENFR);
    if (CUR_TOKEN_IS(LBRACK)){
        PARSE_TOKEN(LBRACK);
        PARSE_TOKEN(RBRACK);
        while(CUR_TOKEN_IS(LBRACK)){
            PARSE_TOKEN(LBRACK);
            PARSE(exp, Exp);
            PARSE_TOKEN(RBRACK);
        }
    }
    log(root, 0);
    return true;
}

// FuncFParams -> FuncFParam { ',' FuncFParam }
bool Parser::parseFuncFParams(FuncFParams* root){
    log(root);
    PARSE(funcFParam, FuncFParam);
    while(CUR_TOKEN_IS(COMMA)){
        PARSE_TOKEN(COMMA);
        PARSE(funcFParam, FuncFParam);
    }
    log(root, 0);
    return true;
}

// Block -> '{' { BlockItem } '}'
bool Parser::parseBlock(Block* root){
    log(root);
    PARSE_TOKEN(LBRACE);
    while(!CUR_TOKEN_IS(RBRACE)){
        PARSE(blockItem, BlockItem);
    }
    #ifdef DEBUG_PARSER
    std::cout << token_stream.size() << ' '<< index << std::endl;
    #endif
    PARSE_TOKEN(RBRACE);
    log(root, 0);
    return true;
}

// BlockItem -> Decl | Stmt
bool Parser::parseBlockItem(BlockItem* root){
    log(root);
    if (CUR_TOKEN_IS(CONSTTK) || CUR_TOKEN_IS(INTTK) || CUR_TOKEN_IS(FLOATTK)){
        PARSE(decl, Decl);
    }
    else{
        PARSE(stmt, Stmt);
    }
    log(root, 0);
    return true;
}

// Stmt ->  LVal '=' Exp ';' |      // Ident
//          Block |                 // LBRACE
//          'if' '(' Cond ')' Stmt [ 'else' Stmt ] |
//          'while' '(' Cond ')' Stmt |
//          'break' ';' |
//          'continue' ';' |
//          'return' [Exp] ';' |
//          [Exp] ';'
bool Parser::parseStmt(Stmt* root){
    log(root);
    if (CUR_TOKEN_IS(IFTK)){
        PARSE_TOKEN(IFTK);
        PARSE_TOKEN(LPARENT);
        PARSE(cond, Cond);
        PARSE_TOKEN(RPARENT);
        PARSE(stmt, Stmt);
        if (CUR_TOKEN_IS(ELSETK)){
            PARSE_TOKEN(ELSETK);
            PARSE(stmt, Stmt);
        }
    }
    else if (CUR_TOKEN_IS(WHILETK)){
        PARSE_TOKEN(WHILETK);
        PARSE_TOKEN(LPARENT);
        PARSE(cond, Cond);
        PARSE_TOKEN(RPARENT);
        PARSE(stmt, Stmt);
    }
    else if (CUR_TOKEN_IS(BREAKTK)){
        PARSE_TOKEN(BREAKTK);
        PARSE_TOKEN(SEMICN);
    }
    else if(CUR_TOKEN_IS(CONTINUETK)){
        PARSE_TOKEN(CONTINUETK);
        PARSE_TOKEN(SEMICN);
    }
    else if(CUR_TOKEN_IS(RETURNTK)){
        PARSE_TOKEN(RETURNTK);
        if (!CUR_TOKEN_IS(SEMICN)){
            PARSE(exp, Exp);
        }
        PARSE_TOKEN(SEMICN);
    }
    else if (CUR_TOKEN_IS(IDENFR) && (CUR_TOKEN_IS_N_STEP(LBRACK, 1) || CUR_TOKEN_IS_N_STEP(ASSIGN, 1))){       // Lval and [Exp] distinction
        PARSE(lval, LVal);
        PARSE_TOKEN(ASSIGN);
        PARSE(exp, Exp);
        PARSE_TOKEN(SEMICN);
    }
    else if(CUR_TOKEN_IS(LBRACE)){
        PARSE(block, Block);
    }
    else{
        if (!CUR_TOKEN_IS(SEMICN)){
            PARSE(exp, Exp);
        }
        PARSE_TOKEN(SEMICN);
    }
    log(root, 0);
    return true;
}

// LVal -> Ident {'[' Exp ']'}
bool Parser::parseLVal(LVal* root){
    log(root);
    PARSE_TOKEN(IDENFR);
    while(CUR_TOKEN_IS(LBRACK)){
        PARSE_TOKEN(LBRACK);
        PARSE(exp, Exp);
        PARSE_TOKEN(RBRACK);
    }
    log(root, 0);
    return true;
}

// AddExp -> MulExp { ('+' | '-') MulExp }
bool Parser::parseAddExp(AddExp* root){
    log(root);
    PARSE(mulExp, MulExp);
    while(CUR_TOKEN_IS(PLUS) || CUR_TOKEN_IS(MINU)){
        if (CUR_TOKEN_IS(PLUS)){
            PARSE_TOKEN(PLUS);
        }
        else{
            PARSE_TOKEN(MINU);
        }
        PARSE(mulExp, MulExp);
    }
    log(root, 0);
    return true;
}

// Cond -> LOrExp
bool Parser::parseCond(Cond* root){
    log(root);
    PARSE(lOrExp, LOrExp);
    log(root, 0);
    return true;
}

// LOrExp -> LAndExp [ '||' LOrExp ]
bool Parser::parseLOrExp(LOrExp* root){
    log(root);
    PARSE(lAndExp, LAndExp);
    if (CUR_TOKEN_IS(OR)){
        PARSE_TOKEN(OR);
        PARSE(lOrExp, LOrExp);
    }
    log(root, 0);
    return true;
}

// Number -> IntConst | floatConst
bool Parser::parseNumber(Number* root){
    log(root);
    if (CUR_TOKEN_IS(INTLTR)){
        PARSE_TOKEN(INTLTR);
    }
    else{
        PARSE_TOKEN(FLOATLTR);
    }
    log(root, 0);
    return true;
}

// PrimaryExp -> '(' Exp ')' | LVal | Number
bool Parser::parsePrimaryExp(PrimaryExp* root){
    log(root);
    if (CUR_TOKEN_IS(LPARENT)){
        PARSE_TOKEN(LPARENT);
        PARSE(exp, Exp);
        PARSE_TOKEN(RPARENT);
    }
    else if (CUR_TOKEN_IS(FLOATLTR) || CUR_TOKEN_IS(INTLTR)){
        PARSE(number, Number);
    }
    else{
        PARSE(lval, LVal);
    }
    log(root, 0);
    return true;
}

// UnaryExp -> PrimaryExp | Ident '(' [FuncRParams] ')' | UnaryOp UnaryExp
bool Parser::parseUnaryExp(UnaryExp* root){
    log(root);
    if (CUR_TOKEN_IS(IDENFR)){
        if (CUR_TOKEN_IS_N_STEP(LPARENT, 1)){
            PARSE_TOKEN(IDENFR);
            PARSE_TOKEN(LPARENT);
            if (!CUR_TOKEN_IS(RPARENT)){
                PARSE(funcRParams, FuncRParams);
            }
            PARSE_TOKEN(RPARENT);
        } else {
            PARSE(primaryExp, PrimaryExp);
        }
    } else if (CUR_TOKEN_IS(PLUS) || CUR_TOKEN_IS(MINU) || CUR_TOKEN_IS(NOT)) {
        PARSE(unaryOp, UnaryOp);
        PARSE(unaryExp, UnaryExp);
    } else {
        PARSE(primaryExp, PrimaryExp);
    }
    log(root, 0);
    return true;
}

// UnaryOp -> '+' | '-' | '!'
bool Parser::parseUnaryOp(UnaryOp* root){
    log(root);
    if (CUR_TOKEN_IS(PLUS)){
        PARSE_TOKEN(PLUS);
    }
    else if (CUR_TOKEN_IS(MINU)){
        PARSE_TOKEN(MINU);
    }
    else if (CUR_TOKEN_IS(NOT)){
        PARSE_TOKEN(NOT);
    }
    else{
        assert(0 && "In unaryop: undesired tk!");
        return false;
    }
    log(root, 0);
    return true;
}

// FuncRParams -> Exp { ',' Exp }
bool Parser::parseFuncRParams(FuncRParams* root){
    log(root);
    PARSE(exp, Exp);
    while(CUR_TOKEN_IS(COMMA)){
        PARSE_TOKEN(COMMA);
        PARSE(exp, Exp);
    }
    log(root, 0);
    return true;
}

// MulExp -> UnaryExp { ('*' | '/' | '%') UnaryExp }
bool Parser::parseMulExp(MulExp* root){
    log(root);
    PARSE(unaryExp, UnaryExp);
    while(CUR_TOKEN_IS(MULT) ||CUR_TOKEN_IS(DIV) ||CUR_TOKEN_IS(MOD)){
        if (CUR_TOKEN_IS(MULT)){
            PARSE_TOKEN(MULT);
        }
        else if (CUR_TOKEN_IS(DIV)){
            PARSE_TOKEN(DIV);
        }
        else if (CUR_TOKEN_IS(MOD)){
            PARSE_TOKEN(MOD);
        }
        PARSE(unaryOp, UnaryExp);
    }
    log(root, 0);
    return true;
}

// RelExp -> AddExp { ('<' | '>' | '<=' | '>=') AddExp }
bool Parser::parseRelExp(RelExp* root){
    log(root);
    PARSE(addExp, AddExp);
    while(CUR_TOKEN_IS(LSS) ||CUR_TOKEN_IS(GTR) ||CUR_TOKEN_IS(LEQ)||CUR_TOKEN_IS(GEQ)){
        if (CUR_TOKEN_IS(LSS)){
            PARSE_TOKEN(LSS);
        }
        else if (CUR_TOKEN_IS(GTR)){
            PARSE_TOKEN(GTR);
        }
        else if (CUR_TOKEN_IS(LEQ)){
            PARSE_TOKEN(LEQ);
        }
        else{
            PARSE_TOKEN(GEQ);
        }
        PARSE(addExp, AddExp);
    }
    log(root, 0);
    return true;
}

// EqExp -> RelExp { ('==' | '!=') RelExp }
bool Parser::parseEqExp(EqExp* root){
    log(root);
    PARSE(relExp, RelExp);
    while(CUR_TOKEN_IS(EQL) ||CUR_TOKEN_IS(NEQ)){
        if (CUR_TOKEN_IS(EQL)){
            PARSE_TOKEN(EQL);
        }
        else{
            PARSE_TOKEN(NEQ);
        }
        PARSE(relExp, RelExp);
    }
    log(root, 0);
    return true;
}

// LAndExp -> EqExp [ '&&' LAndExp ]
bool Parser::parseLAndExp(LAndExp* root){
    log(root);
    PARSE(eqExp, EqExp);
    if (CUR_TOKEN_IS(AND)){
        PARSE_TOKEN(AND);
        PARSE(lAndExp, LAndExp);
    }
    log(root, 0);
    return true;
}

void Parser::log(AstNode* node, int isIn){
#ifdef DEBUG_PARSER
    if (index >= token_stream.size()){
        
    }
    std::string v = token_stream[index].value;
    if (isIn)
    {
        for (int i = 1; i <= DEBUG_NUM; i++)
            std::cout << "| ";
        std::cout << "In parse" << toString(node->type) << ", cur_token_type::" << toString(token_stream[index].type) << ", token_val::" << token_stream[index].value << '\n';
        DEBUG_NUM += 1;
        } else {
            DEBUG_NUM -= 1;
            for (int i = 1; i <= DEBUG_NUM;i++)
                std::cout << "| ";
            std::cout << "Out parse" << toString(node->type) << ", cur_token_type::" << toString(token_stream[index].type) << ", token_val::" << token_stream[index].value << '\n';
        }
#endif
}
