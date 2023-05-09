#include"front/semantic.h"
#include"front/auxiliary_function.h"

#include<cassert>

using ir::Instruction;
using ir::Function;
using ir::Operand;
using ir::Operator;

typedef unsigned int uint;

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
#define TYPE_EQ_INT(node) (node->t == Type::Int)
#define TYPE_EQ_INTLITERAL(node) (node->t == Type::IntLiteral)
#define TYPE_EQ_FLOAT(node) (node->t == Type::Float)
#define TYPE_EQ_FLOATLITERAL(node) (node->t == Type::FloatLiteral)
#define TYPE_EQ_INT_INTLITEARL(node) (TYPE_EQ_INT(node) || TYPE_EQ_INTLITERAL(node))
#define TYPE_EQ_FLOAT_FLOATLITEARL(node) (TYPE_EQ_FLOAT(node) || TYPE_EQ_FLOATLITERAL(node))


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

// struct ScopeInfo {
//     int cnt;    // 作用域在函数中的唯一编号, 代表是函数中出现的第几个作用域
//     string name;    // 分辨作用域的类别, 'b' 代表是一个单独嵌套的作用域, 'i' 'e' 'w' 分别代表由 if else while 产生的新作用域
//     map_str_ste table;      //string 是操作数的【原始】名称, 在 STE 中存放的应该是【变量重命名】后的名称
// };

void frontend::SymbolTable::add_scope(std::string scopeName) {
    ScopeInfo newInfo;
    newInfo.cnt = 0;
    newInfo.name = scopeName;
    map_str_ste newMap;
    newInfo.table = newMap;
    scope_stack.push_back(newInfo);
}

void frontend::SymbolTable::add_scope_entry(STE ste){
    std::string scopeName = get_scoped_name(ste.operand.name);
    int index = scope_stack.size() - 1;
    std::swap(scopeName, ste.operand.name);
    scope_stack[index].table[scopeName] = ste;      // 注意scopeName是原始名字，输入的ste的name本来是原始名字，要交换
}

void frontend::SymbolTable::add_scope_entry(Operand operand, vector<int> dim){
    STE ste;
    ste.dimension = dim;
    ste.operand = operand;
    add_scope_entry(ste);
}

void frontend::SymbolTable::add_scope_entry(Type type, std::string name, vector<int> dim){
    Operand operand(name, type);
    add_scope_entry(operand, dim);
}

void frontend::SymbolTable::exit_scope() {
    scope_stack.pop_back();
}

// 输入一个变量名, 返回其在当前作用域下重命名后的名字 (相当于加后缀)
string frontend::SymbolTable::get_scoped_name(string id) const {
    int scopeNum = this->scope_stack.size();
    string suffix = "_" + std::to_string(scopeNum);     // 注意是从1开始编号
    return id + suffix;
}

//输入一个变量名, 在符号表中寻找最近的同名变量, 返回对应的 Operand
// (注意，此 Operand 的 name 是重命名后的)
Operand frontend::SymbolTable::get_operand(string id) const {
    STE ste = frontend::SymbolTable::get_ste(id);
    return ste.operand;
}

// 注意这个返回的name是重命名后的
std::string frontend::SymbolTable::get_operand_name(string id) const {
    STE ste = frontend::SymbolTable::get_ste(id);
    return ste.operand.name;
}

Type frontend::SymbolTable::get_operand_type(string id) const {
    STE ste = frontend::SymbolTable::get_ste(id);
    return ste.operand.type;
}

// uint frontend::SymbolTable::get_array_offset(std::string id, vector<int> dim) const{
//     STE ste = get_ste(id);
//     assert((ste.operand.type == Type::FloatPtr || ste.operand.type == Type::IntPtr) && "In frontend::SymbolTable::get_array_offset: Unexpected Type");
//     assert((dim.size() == ste.dimension.size()) && "In frontend::SymbolTable::get_array_offset: Unmatched dim size!");
//     uint offset = 0;
//     vector<int> sufProduct(dim.size(), 0);
//     uint index = dim.size() - 1;
//     uint cumProduct = 1;
//     for (int i = index; i >= 0; i--){
//         offset += dim[i] * cumProduct;
//         cumProduct *= ste.dimension[i];
//     }
//     return offset;
//     /* Example:
//      *      declare int a[3][5];
//      *      reference a[2][3];
//      *      offset = 3 * 1 + 2 * 5 = 13
//      * 
//      *      declare int a[3][5][7];
//      *      reference a[2][0][3];
//      *      offset = 3 * 1 + 0 * 5 + 2 * 35 = 73
//      */
// }

//输入一个变量名, 在符号表中寻找最近的同名变量, 返回 STE
frontend::STE frontend::SymbolTable::get_ste(string id) const {
    for (int i = this->scope_stack.size() - 1; i >= 0 ; i--){
        auto it = scope_stack[i].table.find(id);
        if (it != scope_stack[i].table.end()){
            return it->second;
        }
    }
    std::string assertErr = "In frontend::SymbolTable::get_ste: Unexpected id: " + id;
    assert(0 && assertErr.c_str());
}

frontend::Analyzer::Analyzer(): tmp_cnt(0), symbol_table() {
    TODO;
}

ir::Program frontend::Analyzer::get_ir_program(CompUnit* root) {
    TODO;
    ir::Program prog;
    analysisCompUnit(root, prog);
    return prog;
}


// CompUnit -> Decl [CompUnit] | FuncDef [CompUnit]                        // 一个计算单元， 要么是函数声明要么是变量声明
void frontend::Analyzer::analysisCompUnit(CompUnit* root, ir::Program& buffer){

}

// Decl -> ConstDecl | VarDecl                                             // 变量声明，可以是变量也可以是常量
void frontend::Analyzer::analysisDecl(Decl* root, vector<ir::Instruction*>& buffer){

}

// (sName, retType): FuncDef -> FuncType Ident '(' [FuncFParams] ')' Block   // 函数定义：void ident(int params, ...) block
void frontend::Analyzer::analysisFuncDef(FuncDef* root, ir::Function& buffer){

}

// (type): ConstDecl -> 'const' BType ConstDef { ',' ConstDef } ';'                // 如果是常量，有const关键字
void frontend::Analyzer::analysisConstDecl(ConstDecl* root, vector<ir::Instruction*>& buffer){

}

// (type): VarDecl -> BType VarDef { ',' VarDef } ';'                              // 变量定义，int a = ?, b[1][2][3] = {};
void frontend::Analyzer::analysisVarDecl(VarDecl* root, vector<ir::Instruction*>& buffer){

}

// (type): BType -> 'int' | 'float'                                                // BType 是 变量可以有的类型
void frontend::Analyzer::analysisBType(BType* root, vector<ir::Instruction*>& buffer){

}

// (arrName): ConstDef -> Ident { '[' ConstExp ']' } '=' ConstInitVal                 // 常量表达式核心定义，从Ident = Val或Ident[][] = Val
void frontend::Analyzer::analysisConstDef(ConstDef* root, vector<ir::Instruction*>& buffer){

}

// (value, type): ConstInitVal -> ConstExp |'{' [ ConstInitVal { ',' ConstInitVal } ] '}' // 常量表达式值，可能是个数组
void frontend::Analyzer::analysisConstInitVal(ConstInitVal* root, vector<ir::Instruction*>& buffer){

}

// (computable = True, value, type = int): ConstExp -> AddExp
void frontend::Analyzer::analysisConstExp(ConstExp* root, vector<ir::Instruction*>& buffer){

}

// (arrName): VarDef -> Ident { '[' ConstExp ']' } [ '=' InitVal ]                    
void frontend::Analyzer::analysisVarDef(VarDef* root, vector<ir::Instruction*>& buffer){

}

// (computable = False, value, type): InitVal -> Exp | '{' [ InitVal { ',' InitVal } ] '}'
void frontend::Analyzer::analysisInitVal(InitVal* root, vector<ir::Instruction*>& buffer){

}

// (computable = False, value, type): Exp -> AddExp                                           // 加法表达式
void frontend::Analyzer::analysisExp(Exp* root, vector<ir::Instruction*>& buffer){
    ANALYSIS(addExpNode, AddExp, 0)
    COPY_EXP_NODE(addExpNode, root)
}

// FuncType -> 'void' | 'int' | 'float'
void frontend::Analyzer::analysisFuncType(FuncType* root, vector<ir::Instruction*>& buffer){

}

// FuncFParam -> BType Ident ['[' ']' { '[' Exp ']' }]
void frontend::Analyzer::analysisFuncFParam(FuncFParam* root, vector<ir::Instruction*>& buffer){

}

// FuncFParams -> FuncFParam { ',' FuncFParam }            // 逗号表示重复
void frontend::Analyzer::analysisFuncFParams(FuncFParams* root, vector<ir::Instruction*>& buffer){

}

// Block -> '{' { BlockItem } '}'                          // block由多个item构成，每个item以分号结尾，用大括号分隔表示block
void frontend::Analyzer::analysisBlock(Block* root, vector<ir::Instruction*>& buffer){

}

// BlockItem -> Stmt | Decl                                // 每个Item由Statement或者声明构成
void frontend::Analyzer::analysisBlockItem(BlockItem* root, vector<ir::Instruction*>& buffer){

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
void frontend::Analyzer::analysisStmt(Stmt* root, vector<ir::Instruction*>& buffer){

}

// (computable = False, value, type, arrayIndex): LVal -> Ident {'[' Exp ']'}                             // 左值，可能是单个值也可能是数组
void frontend::Analyzer::analysisLVal(LVal* root, vector<ir::Instruction*>& buffer){
    GET_CHILD_PTR(term, Term, 0);
    Token tk = term->token;
    Operand arrOperand = symbol_table.get_operand(tk.value);
    root->v = arrOperand.name;
    if (root->children.size() == 1){        // a variable of Int or Float
        root->t = arrOperand.type;
    }
    // TODO: 测试样例只有二维数组，那么就不弄复杂了，本来是应该算出来的
    else if (root->children.size() == 4){   // 一维数组
        ANALYSIS(expNode1, Exp, 2);
        root->v = "$v_LVal";
        root->t = arrOperand.type == Type::IntPtr ? Type::Int : Type::Float;
        buffer.push_back(new Instruction(arrOperand, Operand(expNode1->v, expNode1->t), Operand(root->v, root->t), Operator::load));
    }
    else{       // 二维数组
        ANALYSIS(expNode1, Exp, 2);
        ANALYSIS(expNode2, Exp, 5);
        root->v = "$v_LVal";
        root->t = arrOperand.type == Type::IntPtr ? Type::Int : Type::Float;
        int dim_2 = symbol_table.get_ste(tk.value).dimension[1];
        buffer.push_back(new Instruction(Operand(std::to_string(dim_2), Type::IntLiteral), Operand(expNode1->v, expNode1->t), Operand("$arrOffset", Type::Int), Operator::mul));
        buffer.push_back(new Instruction(Operand("$arrOffset", Type::Int), Operand(expNode2->v, expNode2->t), Operand("$arrOffset", Type::Int), Operator::add));
        buffer.push_back(new Instruction(arrOperand, Operand("$arrOffset", Type::Int), Operand(root->v, root->t), Operator::load));
    }
}

// (computable = False, value, type): AddExp -> MulExp { ('+' | '-') MulExp }
void frontend::Analyzer::analysisAddExp(AddExp* root, vector<ir::Instruction*>& buffer){
    int index = 0;
    ANALYSIS(mulExpNode_0, MulExp, index);
    COPY_EXP_NODE(mulExpNode_0, root);
    if (root->children.size() != 1){
        root->v = "$v_add";
    }
    while(index + 1 < root->children.size()){
        GET_CHILD_PTR(term, Term, ++index);
        Token tk = term->token;
        ANALYSIS(mulExpNode, MulExp, ++index);
        // 类型转换
        if (TYPE_EQ_FLOAT(mulExpNode) && TYPE_EQ_INT(root)){
            root->t = Type::Float;
            Instruction *inst = new Instruction(Operand(root->v, Type::Int), Operand(), Operand(root->v, root->t), Operator::cvt_i2f);
            buffer.push_back(inst);
        }
        // 子树的类型转换
        if (root->t == Type::Float && mulExpNode->t == Type::Int){
            Instruction *inst = new Instruction(Operand(mulExpNode->v, mulExpNode->t), Operand(), Operand(mulExpNode->v, Type::Float), Operator::cvt_i2f);
            buffer.push_back(inst);
        }
        Instruction *inst = nullptr;
        switch (tk.value[0]){
        case '+':
            if (root->t == Type::Int){
                inst = new Instruction(Operand(root->v, root->t), Operand(mulExpNode->v, mulExpNode->t), Operand(root->v, root->t), Operator::add);
            } else {
                inst = new Instruction(Operand(root->v, root->t), Operand(mulExpNode->v, Type::Float), Operand(root->v, root->t), Operator::fadd);
            }
            break;
        case '-':
            if (root->t == Type::Int){
                inst = new Instruction(Operand(root->v, root->t), Operand(mulExpNode->v, mulExpNode->t), Operand(root->v, root->t), Operator::sub);
            } else {
                inst = new Instruction(Operand(root->v, root->t), Operand(mulExpNode->v, Type::Float), Operand(root->v, root->t), Operator::fsub);
            }
            break;
        default:
            assert(0 && "In analysisAddExp: Unexpected Input!");
        }
        buffer.push_back(inst);
    }
}

// (computable = False, value, type): Cond -> LOrExp                                          
void frontend::Analyzer::analysisCond(Cond* root, vector<ir::Instruction*>& buffer){

}

// (computable = False, value, type = int): LOrExp -> LAndExp [ '||' LOrExp ]
void frontend::Analyzer::analysisLOrExp(LOrExp* root, vector<ir::Instruction*>& buffer){

}

// (computable = True, value, type): Number -> floatConst | IntConst 
void frontend::Analyzer::analysisNumber(Number* root, vector<ir::Instruction*>& buffer){
    GET_CHILD_PTR(term, Term, 0);
    Token tk = term->token;
    root->v = "$literalNum";
    root->is_computable = true;
    if (tk.type == TokenType::FLOATLTR){    // float literal
        root->t = Type::FloatLiteral;
        buffer.push_back(new Instruction(Operand(tk.value, Type::FloatLiteral), Operand(), Operand(root->v, Type::Float), Operator::fdef));
    }
    else{       // int literal
        root->t = Type::IntLiteral;
        buffer.push_back(new Instruction(Operand(tk.value, Type::IntLiteral), Operand(), Operand(root->v, Type::Int), Operator::def));
    }
}

// (computable = False, value, type): PrimaryExp -> '(' Exp ')' | LVal | Number               // 右边表达式的单个项的可能值，可以是括号，数字和变量
void frontend::Analyzer::analysisPrimaryExp(PrimaryExp* root, vector<ir::Instruction*>& buffer){
    if (root->children.size() == 3){    // ( Exp )
        ANALYSIS(expNode, Exp, 1);
        COPY_EXP_NODE(expNode, root);
    }
    else if (root->children[0]->type == NodeType::NUMBER){  // Number
        ANALYSIS(numberNode, Number, 0);
        COPY_EXP_NODE(numberNode, root);
    }
    else{       // LVAL
        ANALYSIS(lvalNode, LVal, 0);
        COPY_EXP_NODE(lvalNode, root);
    }
}

// (computable = False, value, type):  UnaryExp -> PrimaryExp                                  // 单个项
//                                     UnaryExp -> Ident '(' [FuncRParams] ')'                 // 函数
//                                     UnaryExp -> UnaryOp UnaryExp   
void frontend::Analyzer::analysisUnaryExp(UnaryExp* root, vector<ir::Instruction*>& buffer){
    if (root->children.size() == 1){    // Line 1
        ANALYSIS(primaryExp, PrimaryExp, 0);
        COPY_EXP_NODE(primaryExp, root);
    } else if (root->children.size() == 2){     // Line 3
        ANALYSIS(unaryExpNode, UnaryExp, 1);
        COPY_EXP_NODE(unaryExpNode, root);
        GET_CHILD_PTR(term, Term, 0);
        Token tk = term->token;
        switch (tk.value[0])
        {
        case '+':
            break;
        case '-':
            if (root->t == Type::Int){
                buffer.push_back(new Instruction(Operand("0", Type::IntLiteral), Operand(), Operand("$tem0", Type::Int), Operator::def));
                buffer.push_back(new Instruction(Operand("$tem0", Type::Int), Operand(root->v, root->t), Operand(root->v, root->t), Operator::sub));
            } else if (root->t == Type::Float || root->t == Type::FloatLiteral){
                buffer.push_back(new Instruction(Operand("0.0", Type::FloatLiteral), Operand(), Operand("$ftem0", Type::Float), Operator::fdef));
                buffer.push_back(new Instruction(Operand("$ftem0", Type::Float), Operand(root->v, root->t), Operand(root->v, root->t), Operator::fsub));
            } else if (root->t == Type::IntLiteral){
                buffer.push_back(new Instruction(Operand("0", Type::IntLiteral), Operand(), Operand("$tem0", Type::Int), Operator::def));
                buffer.push_back(new Instruction(Operand("$tem0", Type::Int), Operand(root->v, root->t), Operand(root->v, root->t), Operator::subi));
            } else{
                assert(0 && "In analysisUnaryExp: Unexpected Type");
            }
            break;
        case '!':
            buffer.push_back(new Instruction(Operand(root->v, root->t), Operand(), Operand(root->v, root->t), Operator::_not));
            break;
        default:
            break;
        }
    } else {    // Line 2
        GET_CHILD_PTR(term, Term, 0);
        Token tk = term->token;
        std::string funcName = tk.value;
        if (root->children.size() == 4){    // FuncParams
            ANALYSIS(funcParamsNode, FuncRParams, 3);
            TODO;
        }
    }
}

// (TokenType op): UnaryOp -> '+' | '-' | '!'
void frontend::Analyzer::analysisUnaryOp(UnaryOp* root, vector<ir::Instruction*>& buffer){

}

// FuncRParams -> Exp { ',' Exp }                          // 调用函数时的参数
void frontend::Analyzer::analysisFuncRParams(FuncRParams* root, vector<ir::Instruction*>& buffer){
    
}

// (computable = False, value, type): MulExp -> UnaryExp { ('*' | '/' | '%') UnaryExp }                     
void frontend::Analyzer::analysisMulExp(MulExp* root, vector<ir::Instruction*>& buffer){
    int index = 0;
    ANALYSIS(unaryExpNode_0, UnaryExp, index);
    COPY_EXP_NODE(unaryExpNode_0, root);
    if (root->children.size() != 1){
        root->v = "$v_mul";
    }
    while (index + 1 < root->children.size())
    {
        GET_CHILD_PTR(term, Term, ++index);
        Token tk = term->token;
        ANALYSIS(unaryExpNode, UnaryExp, ++index);
        // 根类型修改
        if (TYPE_EQ_INT(root) && TYPE_EQ_FLOAT(unaryExpNode)){
            root->t = Type::Float;
            buffer.push_back(new Instruction(Operand(root->v, Type::Int), Operand(), Operand(root->v, root->t), Operator::cvt_i2f));
        }
        // 子树类型修改，根变成float后，子树操作数也是float
        if (TYPE_EQ_FLOAT(root) && TYPE_EQ_INT(unaryExpNode)){
            buffer.push_back(new Instruction(Operand(unaryExpNode->v, unaryExpNode->t), Operand(), Operand(unaryExpNode->v, Type::Float), Operator::cvt_i2f));
        }
        Instruction* inst = nullptr;
        switch (tk.value[0])
        {
        case '*':
            if (TYPE_EQ_INT(root)){
                inst = new Instruction(Operand(root->v, root->t), Operand(unaryExpNode->v, unaryExpNode->t), Operand(root->v, root->t), Operator::mul);
            } else {
                inst = new Instruction(Operand(root->v, root->t), Operand(unaryExpNode->v, Type::Float), Operand(root->v, root->t), Operator::mul);
            }
            break;
        case '/':
            if (TYPE_EQ_INT(root)){
                inst = new Instruction(Operand(root->v, root->t), Operand(unaryExpNode->v, unaryExpNode->t), Operand(root->v, root->t), Operator::div);
            } else {
                inst = new Instruction(Operand(root->v, root->t), Operand(unaryExpNode->v, Type::Float), Operand(root->v, root->t), Operator::fdiv);
            }
            break;
        case '%':           // 说是整型变量取余，那就不考虑了。
            inst = new Instruction(root->v, unaryExpNode->v, root->v, Operator::mod);
            break;
        default:
            assert(0 && "In analysisMulExp: Unexpected Input!");
        }
        buffer.push_back(inst);
    }
}

// (computable = False, value, type = int): RelExp -> AddExp { ('<' | '>' | '>=' | '<=') AddExp }
void frontend::Analyzer::analysisRelExp(RelExp* root, vector<ir::Instruction*>& buffer){

}

// (computable = False, value, type = int): EqExp -> RelExp { ('==' | '!=') RelExp }
void frontend::Analyzer::analysisEqExp(EqExp* root, vector<ir::Instruction*>& buffer){

}

// (computable = False, value, type = int): LAndExp -> EqExp [ '&&' LAndExp ]
void frontend::Analyzer::analysisLAndExp(LAndExp* root, vector<ir::Instruction*>& buffer){

}
