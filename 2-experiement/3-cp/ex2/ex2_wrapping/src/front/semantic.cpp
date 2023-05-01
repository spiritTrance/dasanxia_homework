#include"front/semantic.h"

#include<cassert>

using ir::Instruction;
using ir::Function;
using ir::Operand;
using ir::Operator;

#define TODO assert(0 && "TODO");

#define GET_CHILD_PTR(node, type, index) \
    auto node = dynamic_cast<type*>(root->children[index]);\
    assert(node); 
#define ANALYSIS(node, type, index) \
    auto node = dynamic_cast<type*>(root->children[index]); \
    assert(node); \
    analysis##type(node, buffer);
#define COPY_EXP_NODE(from, to) \
    to->is_computable = from->is_computable; \
    to->v = from->v; \
    to->t = from->t;

map<std::string,ir::Function*>* frontend::get_lib_funcs() {
    static map<std::string,ir::Function*> lib_funcs = {
        {"getint", new Function("getint", Type::Int)},
        {"getch", new Function("getch", Type::Int)},
        {"getfloat", new Function("getfloat", Type::Float)},
        {"getarray", new Function("getarray", {Operand("arr", Type::IntPtr)}, Type::Int)},
        {"getfarray", new Function("getfarray", {Operand("arr", Type::FloatPtr)}, Type::Int)},
        {"putint", new Function("putint", {Operand("i", Type::Int)}, Type::null)},
        {"putch", new Function("putch", {Operand("i", Type::Int)}, Type::null)},
        {"putfloat", new Function("putfloat", {Operand("f", Type::Float)}, Type::null)},
        {"putarray", new Function("putarray", {Operand("n", Type::Int), Operand("arr", Type::IntPtr)}, Type::null)},
        {"putfarray", new Function("putfarray", {Operand("n", Type::Int), Operand("arr", Type::FloatPtr)}, Type::null)},
    };
    return &lib_funcs;
}

void frontend::SymbolTable::add_scope(Block* node) {
    TODO;
}
void frontend::SymbolTable::exit_scope() {
    // TODO;
    scope_stack.pop_back();
}


string frontend::SymbolTable::get_scoped_name(string id) const {
    // TODO; exist questions
    return id + "_block";
}

Operand frontend::SymbolTable::get_operand(string id) const {
    // TODO;
    
}

frontend::STE frontend::SymbolTable::get_ste(string id) const {
    TODO;
}

frontend::Analyzer::Analyzer(): tmp_cnt(0), symbol_table() {
    // TODO;
}

ir::Program frontend::Analyzer::get_ir_program(CompUnit* root) {
    // TODO;
    ir::Program prog;
    analysisCompUnit(root, prog);
    return prog;
}

// CompUnit -> Decl [CompUnit] | FuncDef [CompUnit]                        // 一个计算单元， 要么是函数声明要么是变量声明
void frontend::Analyzer::analysisCompUnit(CompUnit* node, ir::Program& buffer){

}

// Decl -> ConstDecl | VarDecl                                             // 变量声明，可以是变量也可以是常量
void frontend::Analyzer::analysisDecl(Decl* node, ir::Program& buffer){

}

// (sName, retType): FuncDef -> FuncType Ident '(' [FuncFParams] ')' Block   // 函数定义：void ident(int params, ...) block
void frontend::Analyzer::analysisFuncDef(FuncDef* node, ir::Program& buffer){

}

// (type): ConstDecl -> 'const' BType ConstDef { ',' ConstDef } ';'                // 如果是常量，有const关键字
void frontend::Analyzer::analysisConstDecl(ConstDecl* node, ir::Program& buffer){

}

// (type): VarDecl -> BType VarDef { ',' VarDef } ';'                              // 变量定义，int a = ?, b[1][2][3] = {};
void frontend::Analyzer::analysisVarDecl(VarDecl* node, ir::Program& buffer){

}

// (type): BType -> 'int' | 'float'                                                // BType 是 变量可以有的类型
void frontend::Analyzer::analysisBType(BType* node, ir::Program& buffer){

}

// (arrName): ConstDef -> Ident { '[' ConstExp ']' } '=' ConstInitVal                 // 常量表达式核心定义，从Ident = Val或Ident[][] = Val
void frontend::Analyzer::analysisConstDef(ConstDef* node, ir::Program& buffer){

}

// (value, type): ConstInitVal -> ConstExp |'{' [ ConstInitVal { ',' ConstInitVal } ] '}' // 常量表达式值，可能是个数组
void frontend::Analyzer::analysisConstInitVal(ConstInitVal* node, ir::Program& buffer){

}

// (computable = True, value, type = int): ConstExp -> AddExp
void frontend::Analyzer::analysisConstExp(ConstExp* node, ir::Program& buffer){

}

// (arrName): VarDef -> Ident { '[' ConstExp ']' } [ '=' InitVal ]                    
void frontend::Analyzer::analysisVarDef(VarDef* node, ir::Program& buffer){

}

// (computable = False, value, type): InitVal -> Exp | '{' [ InitVal { ',' InitVal } ] '}'
void frontend::Analyzer::analysisInitVal(InitVal* node, ir::Program& buffer){

}

// (computable = False, value, type): Exp -> AddExp                                           // 加法表达式
void frontend::Analyzer::analysisExp(Exp* node, ir::Program& buffer){

}

// FuncType -> 'void' | 'int' | 'float'
void frontend::Analyzer::analysisFuncType(FuncType* node, ir::Program& buffer){

}

// FuncFParam -> BType Ident ['[' ']' { '[' Exp ']' }]
void frontend::Analyzer::analysisFuncFParam(FuncFParam* node, ir::Program& buffer){

}

// FuncFParams -> FuncFParam { ',' FuncFParam }            // 逗号表示重复
void frontend::Analyzer::analysisFuncFParams(FuncFParams* node, ir::Program& buffer){

}

// Block -> '{' { BlockItem } '}'                          // block由多个item构成，每个item以分号结尾，用大括号分隔表示block
void frontend::Analyzer::analysisBlock(Block* node, ir::Program& buffer){

}

// BlockItem -> Stmt | Decl                                // 每个Item由Statement或者声明构成
void frontend::Analyzer::analysisBlockItem(BlockItem* node, ir::Program& buffer){

}

// ????(jump_eow, jump_bow):   Stmt -> LVal '=' Exp ';'                                // 对一个已有变量的赋值
//                             Stmt -> Block                                           // 又是一个block
//                             Stmt -> 'if' '(' Cond ')' Stmt [ 'else' Stmt ]          // if else
//                             Stmt -> 'while' '(' Cond ')' Stmt
//                             Stmt -> 'break' ';'
//                             Stmt -> 'continue' ';'
//                             Stmt -> 'return' [Exp] ';'
//                             Stmt -> Exp ';'
//                             Stmt -> ';'
void frontend::Analyzer::analysisStmt(Stmt* node, ir::Program& buffer){

}

// (computable = False, value, , arrayIndex): LVal -> Ident {'[' Exp ']'}                             // 左值，可能是单个值也可能是数组
void frontend::Analyzer::analysisLVal(LVal* node, ir::Program& buffer){

}

// (computable = False, value, type): AddExp -> MulExp { ('+' | '-') MulExp }
void frontend::Analyzer::analysisAddExp(AddExp* node, ir::Program& buffer){

}

// (computable = False, value, type): Cond -> LOrExp                                          
void frontend::Analyzer::analysisCond(Cond* node, ir::Program& buffer){

}

// (computable = False, value, type = int): LOrExp -> LAndExp [ '||' LOrExp ]
void frontend::Analyzer::analysisLOrExp(LOrExp* node, ir::Program& buffer){

}

// (computable = True, value, type): Number -> floatConst | IntConst 
void frontend::Analyzer::analysisNumber(Number* node, ir::Program& buffer){

}

// (computable = False, value, type): PrimaryExp -> '(' Exp ')' | LVal | Number               // 右边表达式的单个项的可能值，可以是括号，数字和变量
void frontend::Analyzer::analysisPrimaryExp(PrimaryExp* node, ir::Program& buffer){

}

// (computable = False, value, type):  UnaryExp -> PrimaryExp                                  // 单个项
//                                     UnaryExp -> Ident '(' [FuncRParams] ')'                 // 函数
//                                     UnaryExp -> UnaryOp UnaryExp   
void frontend::Analyzer::analysisUnaryExp(UnaryExp* node, ir::Program& buffer){

}

// (TokenType op): UnaryOp -> '+' | '-' | '!'
void frontend::Analyzer::analysisUnaryOp(UnaryOp* node, ir::Program& buffer){

}

// FuncRParams -> Exp { ',' Exp }                          // 调用函数时的参数
void frontend::Analyzer::analysisFuncRParams(FuncRParams* node, ir::Program& buffer){

}

// (computable = False, value, type): MulExp -> UnaryExp { ('*' | '/' | '%') UnaryExp }                     
void frontend::Analyzer::analysisMulExp(MulExp* node, ir::Program& buffer){

}

// (computable = False, value, type = int): RelExp -> AddExp { ('<' | '>' | '>=' | '<=') AddExp }
void frontend::Analyzer::analysisRelExp(RelExp* node, ir::Program& buffer){

}

// (computable = False, value, type = int): EqExp -> RelExp { ('==' | '!=') RelExp }
void frontend::Analyzer::analysisEqExp(EqExp* node, ir::Program& buffer){

}

// (computable = False, value, type = int): LAndExp -> EqExp [ '&&' LAndExp ]
void frontend::Analyzer::analysisLAndExp(LAndExp* node, ir::Program& buffer){

}
