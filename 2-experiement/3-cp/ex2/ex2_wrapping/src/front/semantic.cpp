#include"front/semantic.h"
#include"front/auxiliary_function.h"

#include<cassert>

using frontend::Token;
using ir::Function;
using ir::Instruction;
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
// 类型系统判等宏定义
#define TYPE_EQ(node, tp) (node->t == tp)
#define TYPE_EQ_INT(node) TYPE_EQ(node, Type::Int)
#define TYPE_EQ_INTLITERAL(node) TYPE_EQ(node, Type::IntLiteral)
#define TYPE_EQ_FLOAT(node) TYPE_EQ(node, Type::Float)
#define TYPE_EQ_FLOATLITERAL(node) TYPE_EQ(node, Type::FloatLiteral)
#define TYPE_EQ_INT_INTLITEARL(node) (TYPE_EQ_INT(node) || TYPE_EQ_INTLITERAL(node))
#define TYPE_EQ_FLOAT_FLOATLITEARL(node) (TYPE_EQ_FLOAT(node) || TYPE_EQ_FLOATLITERAL(node))
#define TYPE_EQ_I_LTR_PTR(node) ((TYPE_EQ_INT(node)) || (TYPE_EQ_INTLITERAL(node)) || (TYPE_EQ(node, Type::IntPtr)))
#define TYPE_EQ_F_LTR_PTR(node) ((TYPE_EQ_FLOAT(node)) || (TYPE_EQ_FLOATLITERAL(node)) || (TYPE_EQ(node, Type::FloatPtr)))
#define TYPE_EQ_PTR(node) (TYPE_EQ(node, Type::IntPtr) || TYPE_EQ(node, Type::FloatPtr))
#define TYPE_EQ_LITERAL(node) (TYPE_EQ(node, Type::IntLiteral) || TYPE_EQ(node, Type::FloatLiteral))
#define TYPE_EQ_VAR(node) (TYPE_EQ(node, Type::Float) || TYPE_EQ(node, Type::Int))
// 其他乱七八糟的
#define OPERAND_NODE(node) (Operand(node->v, node->t))
#define NODEPTR_CAST(type) dynamic_cast<type*>
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

void frontend::SymbolTable::add_scope_entry(STE ste, bool isFunc = false){
    // 注意现在STE存的值没有重命名，如果是其他标识符，则会全部进行重命名
    if (isFunc){
        std::string scopeName = ste.operand.name;
        scope_stack[0].table[scopeName] = ste;
        return;
    }
    std::string scopeName = get_scoped_name(ste.operand.name);
    int index = scope_stack.size() - 1;
    std::swap(scopeName, ste.operand.name);     // 重命名交换到STE里面去，未重命名的作为key
    scope_stack[index].table[scopeName] = ste;      // 注意scopeName是原始名字，输入的ste的name本来是原始名字，要交换
}

void frontend::SymbolTable::add_scope_entry(Operand operand, vector<int> dim, bool isFunc = false){
    STE ste;
    ste.dimension = dim;
    ste.operand = operand;
    add_scope_entry(ste, isFunc);
}

void frontend::SymbolTable::add_scope_entry(Type type, std::string name, vector<int> dim, bool isFunc = false){
    Operand operand(name, type);
    add_scope_entry(operand, dim, isFunc);
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
Operand frontend::SymbolTable::get_operand(string id, bool isFunc = false) const {
    STE ste = frontend::SymbolTable::get_ste(id, isFunc);
    return ste.operand;
}

// 注意这个返回的name是重命名后的
std::string frontend::SymbolTable::get_operand_name(string id, bool isFunc = false) const {
    STE ste = frontend::SymbolTable::get_ste(id, isFunc);
    return ste.operand.name;
}

Type frontend::SymbolTable::get_operand_type(string id, bool isFunc = false) const {
    STE ste = frontend::SymbolTable::get_ste(id, isFunc);
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
frontend::STE frontend::SymbolTable::get_ste(string id, bool isFunc = false) const {
    if (isFunc){        // 查找的是函数名
        auto it = scope_stack[0].table.find(id);
        assert(it != scope_stack[0].table.end() && "In frontend::SymbolTable::get_ste: Invalid FuncName!");
        return it->second;
    }
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
    // 加入函数的scope和全局变量的Scope
    symbol_table.add_scope("__$FuncScope");
    symbol_table.add_scope("__$GlobalScope");
    analysisCompUnit(root, prog);
    // 加全局变量
    ScopeInfo& globalValInfo = symbol_table.scope_stack[1];
    for (auto it: globalValInfo.table){
        const Operand operand = it.second.operand;
        vector<int>& dim = it.second.dimension;
        int maxLen = 1;
        for (int i: dim){
            maxLen *= i;
        }
        if (dim.empty()){
            ir::GlobalVal globalVal(operand);
            prog.globalVal.push_back(globalVal);
        }
        else{
            ir::GlobalVal globalVal(operand, maxLen);
            prog.globalVal.push_back(globalVal);
        }
    }
    return prog;
}


// CompUnit -> Decl [CompUnit] | FuncDef [CompUnit]                        // 一个计算单元， 要么是函数声明要么是变量声明
void frontend::Analyzer::analysisCompUnit(CompUnit* root, ir::Program& buffer){
    ir::Program &prog = buffer;
    GET_CHILD_PTR(son, AstNode, 0);
    if (son->type == NodeType::DECL){           // 可能是定义变量
        // 注意这里Local有个buffer，就很头疼，怎么区分啊
        vector<ir::Instruction *> buffer;
        ANALYSIS(decl, Decl, 0);
    }
    else{               // 函数定义
        ir::Function buffer;
        ANALYSIS(decl, FuncDef, 0);
        prog.addFunction(buffer);       // prog加函数
    }
    if (root->children.size()!=1){
        ANALYSIS(son2, CompUnit, 1);
    }
}

// Decl -> ConstDecl | VarDecl                                             // 变量声明，可以是变量也可以是常量
void frontend::Analyzer::analysisDecl(Decl* root, vector<ir::Instruction*>& buffer){
    if (root->children[0]->type == NodeType::CONSTDECL){
        ANALYSIS(constDecl, ConstDecl, 0);
    } else {
        ANALYSIS(varDecl, VarDecl, 0);
    }
}

// (sName, retType): FuncDef -> FuncType Ident '(' [FuncFParams] ')' Block   // 函数定义：void ident(int params, ...) block
void frontend::Analyzer::analysisFuncDef(FuncDef* root, ir::Function& buffer){
    GET_CHILD_PTR(funcType, FuncType, 0);
    Token retTypeTk = analysisFuncType(funcType);
    GET_CHILD_PTR(ident, Term, 0);
    // 返回参数
    buffer.returnType = retTypeTk.type == TokenType::VOIDTK ? Type::null :
                        retTypeTk.type == TokenType::INTTK ? Type::Int : Type::Float;
    // 函数名
    buffer.name = ident->token.value;
    // 添加形参列表
    if (root->children.size() == 6){        // have FuncFParams, getParamList
        GET_CHILD_PTR(funcFParams, FuncFParams, 3);
        buffer.ParameterList = analysisFuncFParams(funcFParams);
    }
    // 注意Block要添加了
    symbol_table.add_scope(ident->token.value);
    ir::Function &func = buffer;
    //  利用作用域解决buffer的宏定义问题
    {
        vector<ir::Instruction *> buffer;
        int childNum = root->children.size();
        ANALYSIS(block, Block, childNum - 1);
        func.InstVec = buffer;
    }
    symbol_table.exit_scope();
}

// (type): ConstDecl -> 'const' BType ConstDef { ',' ConstDef } ';'                // 如果是常量，有const关键字
void frontend::Analyzer::analysisConstDecl(ConstDecl* root, vector<ir::Instruction*>& buffer){
    GET_CHILD_PTR(btype, BType, 1);
    Token tk = analysisBType(btype);
    root->t = tk.type == TokenType::INTTK ? Type::Int : Type::Float;
    ANALYSIS(constDef, ConstDef, 2);
    size_t index = 4;
    while (index <= root->children.size() - 1)
    {
        ANALYSIS(constDef, ConstDef, index);
        index += 2;
    }
}

// (type): VarDecl -> BType VarDef { ',' VarDef } ';'                              // 变量定义，int a = ?, b[1][2][3] = {};
void frontend::Analyzer::analysisVarDecl(VarDecl* root, vector<ir::Instruction*>& buffer){
    GET_CHILD_PTR(btype, BType, 0);
    Token tk = analysisBType(btype);
    root->t = tk.type == TokenType::INTTK ? Type::Int : Type::Float;
    ANALYSIS(varDef, VarDef, 2);
    size_t index = 3;
    while (index <= root->children.size() - 1)
    {
        ANALYSIS(varDef, VarDef, index);
        index += 2;
    }
}

// (type): BType -> 'int' | 'float'                                                // BType 是 变量可以有的类型
Token frontend::Analyzer::analysisBType(BType* root){
    GET_CHILD_PTR(term, Term, 0);
    return term->token;
}

// (arrName): ConstDef -> Ident { '[' ConstExp ']' } '=' ConstInitVal                 // 常量表达式核心定义，从Ident = Val或Ident[][] = Val
void frontend::Analyzer::analysisConstDef(ConstDef* root, vector<ir::Instruction*>& buffer){
    Type tp = dynamic_cast<ConstDecl*>(root->parent)->t;
    assert((tp == Type::Int || tp == Type::Float) && "Invalid type in frontend::Analyzer::analysisConstDef");
    GET_CHILD_PTR(ident, Term, 0);
    std::string sVarName = ident->token.value;
    root->arr_name = sVarName;      // 存的是原来的值，只是翻译的时候要重命名
    /* 这里要做一下思考：
     * 如果是Int/Float，需要def/fdef;
     * 如果是Ptr，则需要求出dimension并alloc;
     * 那么，我们的策略是：
     * 在ConstDef结点，执行def/fdef和alloc，那么：
     * 对于Int/Float，行为是让ConstInitVal把值给传上来，赋值，因此赋值在COnstDef中
     * 但对于数组呢？会使用Initializer{}来赋值，显然不能通过节点type来赋值
     * 那么我们的策略是在ConstInitVal赋值
     * 问题来了，ConstInitVal里面怎么做？注意其父节点类型仍然可能是自己。
     * ================================
     * 对于ConstInitVal节点的操作：
     * 查看父节点的类型，如果是ConstDef：
     *                  查看ArrName种类，如果是Int，则直接通过属性传值（注意仍然可能是六种情况）
     *                                  如果是ptr，那么获取dimension，开始store进行赋值（如何求下标赋值是个值得注意的问题）
     *                  其他情况，直接通过属性传值即可（仍然注意六种情况）
     *      - def/fdef
     *      - 求dim并赋值
     *      - 维护符号表
     */

    if (root->children.size() == 3){            // Var
        vector<int> temp;
        symbol_table.add_scope_entry(tp, sVarName, temp);   // 加入符号表
        ANALYSIS(constInitVal, ConstInitVal, 2);
        Operand initVarOperand(symbol_table.get_scoped_name(sVarName), tp);
        if (tp == Type::Int){       // 传上来的是六种情况之一
            if (constInitVal->t == Type::IntLiteral){
                buffer.push_back(new Instruction(OPERAND_NODE(constInitVal), Operand(), initVarOperand, Operator::def));
            }
            else{
                Operand cvtOperand = castExpectedType(OPERAND_NODE(constInitVal), Type::Int, buffer);
                buffer.push_back(new Instruction(cvtOperand, Operand(), initVarOperand, Operator::mov));
            }
        }
        else{       // Float
            if (constInitVal->t == Type::FloatLiteral){
                buffer.push_back(new Instruction(OPERAND_NODE(constInitVal), Operand(), initVarOperand, Operator::fdef));
            }
            else{
                Operand cvtOperand = castExpectedType(OPERAND_NODE(constInitVal), Type::Float, buffer);
                buffer.push_back(new Instruction(cvtOperand, Operand(), initVarOperand, Operator::mov));
            }
        }
        return;
    }
    // Ptr, 只要能想到int arr[2][2] = {{1,1}, {1,1}}就能想到为什么这个节点有个arrName了，定义语句放在ConstInitVal了, 同时类型可以在符号表里面去查了
    tp == Type::Int ? Type::IntPtr : Type::FloatPtr;
    if (root->children.size() == 6){       // 1D array: a [ 9 ] = 9
        // TODO BUG : 根据样例，定义的时候，数组大小的Literal，我认为是Literal，有BUG再改
        ANALYSIS(constExp, ConstExp, 2);
        assert("constExp unprocessed: you need process is_computable");
        TODO;
        assert(TYPE_EQ_LITERAL(constExp) && "In frontend::Analyzer::analysisConstDef: Not a Literal");
        int val = std::stoi(constExp->v);
        vector<int> dim = {val};
        buffer.push_back(new Instruction(OPERAND_NODE(constExp), Operand(), Operand(symbol_table.get_scoped_name(sVarName), tp), Operator::alloc));
        symbol_table.add_scope_entry(tp, sVarName, dim);
        ANALYSIS(constInitVal, ConstInitVal, 5);
        return;
    } 
    else if (root->children.size() == 9){          // 2D array
        ANALYSIS(constExp_1, ConstExp, 2);
        ANALYSIS(constExp_2, ConstExp, 5);
        assert((TYPE_EQ_LITERAL(constExp_2)&&TYPE_EQ_LITERAL(constExp_1)) && "In frontend::Analyzer::analysisConstDef: Not a Literal");
        int val_1 = std::stoi(constExp_1->v);
        int val_2 = std::stoi(constExp_2->v);
        vector<int> dim = {val_1, val_2};
        int tot = val_1 * val_2;
        buffer.push_back(new Instruction(Operand(std::to_string(tot), Type::IntLiteral), Operand(), Operand(symbol_table.get_scoped_name(sVarName), tp), Operator::alloc));
        symbol_table.add_scope_entry(tp, sVarName, dim);
        ANALYSIS(constInitVal, ConstInitVal, 8);
        return;
    }
    else{
        assert("In frontend::Analyzer::analysisConstDef: Unexpected Children Size!");
    }
}

// (value, type): ConstInitVal -> ConstExp |'{' [ ConstInitVal { ',' ConstInitVal } ] '}' // 常量表达式值，可能是个数组
void frontend::Analyzer::analysisConstInitVal(ConstInitVal* root, vector<ir::Instruction*>& buffer){
    // 看父节点类型，是COnstDef再考虑是否赋值，否则只管求值即可?Alloc如何看？
    // 逻辑：父节点是数组则负责赋值，否则节点赋值
    // 查了下用例，{}分支只可能是数组初始赋值，但是{{},{}}要小心考虑
    AstNode *fa = root->parent;
    if (fa->type == NodeType::CONSTDEF){
        ConstDef* parent = dynamic_cast<ConstDef*>(fa);
        std::string arrName = parent->arr_name;
        STE arrSte = symbol_table.get_ste(arrName);
        Type tp = arrSte.operand.type;
        std::string arrScopedName = arrSte.operand.name;
        if (tp == Type::IntPtr || tp == Type::FloatPtr){        // 数组赋值，根据SysY文档，不可能出现{{1,2},{3,4}}的情况
            vector<int> dim = symbol_table.get_ste(arrName).dimension;
            int sz = 1;
            for (int i: dim){
                sz *= i;
            }
            if (root->children.size() == 2){
                for (int i = 0; i < sz;i++){
                    buffer.push_back(new Instruction(Operand(arrScopedName, tp), Operand(std::to_string(i), Type::IntLiteral), Operand("0", Type::IntLiteral), Operator::store));
                }
            }
            else{
                int childSize = root->children.size();
                assert((sz * 2 + 1 == childSize) && "In frontend::Analyzer::analysisConstInitVal: Wrong Size of Initializer");
                for (int i = 1; i < childSize; i += 2){
                    ANALYSIS(constInitVal, ConstInitVal, i);
                    assert(TYPE_EQ_LITERAL(constInitVal) && "In frontend::Analyzer::analysisConstInitVal: Not a Literal in initializing array");
                    Type targetedTp = tp == Type::IntPtr ? Type::Int : Type::Float;
                    buffer.push_back(new Instruction(Operand(arrScopedName, tp), Operand(std::to_string(i / 2), Type::IntLiteral), Operand(constInitVal->v, targetedTp), Operator::store));
                }
            }
            return;
        }
    }
    // 只赋值
    ANALYSIS(constExp, ConstExp, 0);
    assert(TYPE_EQ_LITERAL(constExp) && "In frontend::Analyzer::analysisConstInitVal: Not a Literal in analyse constExp");
    root->v = constExp->v;
    root->t = constExp->t;
    return;
}

// (computable = True, value, type = int): ConstExp -> AddExp
void frontend::Analyzer::analysisConstExp(ConstExp* root, vector<ir::Instruction*>& buffer){
    ANALYSIS(constExp, ConstExp, 0);
    COPY_EXP_NODE(constExp, root);
}

// (arrName): VarDef -> Ident { '[' ConstExp ']' } [ '=' InitVal ]                    
void frontend::Analyzer::analysisVarDef(VarDef* root, vector<ir::Instruction*>& buffer){
    Type tp = dynamic_cast<VarDecl*>(root->parent)->t;
    assert((tp == Type::Int || tp == Type::Float) && "Invalid type in frontend::Analyzer::analysisVarDef");
    GET_CHILD_PTR(ident, Term, 0);
    std::string sVarName = ident->token.value;
    root->arr_name = sVarName;      // 存的是原来的值，只是翻译的时候要重命名
    // 这里要考虑未初始化的情况了！！！！！
    int childNum = root->children.size();
    GET_CHILD_PTR(pendingInitVal, AstNode, childNum - 1);
    if (pendingInitVal->type == NodeType::INITVAL){     // 已初始化
        if (root->children.size() == 3){            // Var
            vector<int> temp;
            symbol_table.add_scope_entry(tp, sVarName, temp);   // 加入符号表
            ANALYSIS(initVal, InitVal, 2);
            Operand initVarOperand(symbol_table.get_scoped_name(sVarName), tp);
            if (tp == Type::Int){       // 传上来的是六种情况之一
                if (initVal->t == Type::IntLiteral){
                    buffer.push_back(new Instruction(OPERAND_NODE(initVal), Operand(), initVarOperand, Operator::def));
                }
                else{
                    Operand cvtOperand = castExpectedType(OPERAND_NODE(initVal), Type::Int, buffer);
                    buffer.push_back(new Instruction(cvtOperand, Operand(), initVarOperand, Operator::mov));
                }
            }
            else{       // Float
                if (initVal->t == Type::FloatLiteral){
                    buffer.push_back(new Instruction(OPERAND_NODE(initVal), Operand(), initVarOperand, Operator::fdef));
                }
                else{
                    Operand cvtOperand = castExpectedType(OPERAND_NODE(initVal), Type::Float, buffer);
                    buffer.push_back(new Instruction(cvtOperand, Operand(), initVarOperand, Operator::mov));
                }
            }
            return;
        }
        // Ptr, 只要能想到int arr[2][2] = {{1,1}, {1,1}}就能想到为什么这个节点有个arrName了，定义语句放在ConstInitVal了, 同时类型可以在符号表里面去查了
        tp == Type::Int ? Type::IntPtr : Type::FloatPtr;
        if (root->children.size() == 6){       // 1D array: a [ 9 ] = 9
            // TODO BUG : 根据样例，定义的时候，数组大小的Literal，我认为是Literal，有BUG再改
            ANALYSIS(constExp, ConstExp, 2);
            assert("constExp unprocessed: you need process is_computable");
            TODO;
            assert(TYPE_EQ_LITERAL(constExp) && "In frontend::Analyzer::analysisVarDef: Not a Literal");
            int val = std::stoi(constExp->v);
            vector<int> dim = {val};
            buffer.push_back(new Instruction(OPERAND_NODE(constExp), Operand(), Operand(symbol_table.get_scoped_name(sVarName), tp), Operator::alloc));
            symbol_table.add_scope_entry(tp, sVarName, dim);
            ANALYSIS(initVal, InitVal, 5);
            return;
        } 
        else if (root->children.size() == 9){          // 2D array
            ANALYSIS(constExp_1, ConstExp, 2);
            ANALYSIS(constExp_2, ConstExp, 5);
            assert((TYPE_EQ_LITERAL(constExp_2)&&TYPE_EQ_LITERAL(constExp_1)) && "In frontend::Analyzer::analysisVarDef: Not a Literal");
            int val_1 = std::stoi(constExp_1->v);
            int val_2 = std::stoi(constExp_2->v);
            vector<int> dim = {val_1, val_2};
            int tot = val_1 * val_2;
            buffer.push_back(new Instruction(Operand(std::to_string(tot), Type::IntLiteral), Operand(), Operand(symbol_table.get_scoped_name(sVarName), tp), Operator::alloc));
            symbol_table.add_scope_entry(tp, sVarName, dim);
            ANALYSIS(initVal, InitVal, 8);
            return;
        }
        else{
            assert("In frontend::Analyzer::analysisVarDef: Unexpected Children Size");
        }
    }
    else{       // 未初始化
        if (root->children.size() == 1){
            vector<int> temp;
            symbol_table.add_scope_entry(tp, sVarName, temp);   // 加入符号表
            if (tp == Type::Int){       // 传上来的是六种情况之一
                buffer.push_back(new Instruction(Operand("0",Type::IntLiteral), Operand(), Operand(symbol_table.get_scoped_name(sVarName), tp), Operator::def));
            }
            else{       // Float
                buffer.push_back(new Instruction(Operand("0.0",Type::FloatLiteral), Operand(), Operand(symbol_table.get_scoped_name(sVarName), tp), Operator::fdef));
            }
            return;
        }
        else if (root->children.size() == 4){
            ANALYSIS(constExp, ConstExp, 2);
            assert("constExp unprocessed: you need process is_computable");
            TODO;
            assert(TYPE_EQ_LITERAL(constExp) && "In frontend::Analyzer::analysisVarDef: Not a Literal");
            int val = std::stoi(constExp->v);
            vector<int> dim = {val};
            buffer.push_back(new Instruction(OPERAND_NODE(constExp), Operand(), Operand(symbol_table.get_scoped_name(sVarName), tp), Operator::alloc));
            symbol_table.add_scope_entry(tp, sVarName, dim);
            return;
        }
        else if (root->children.size() == 7){
            ANALYSIS(constExp_1, ConstExp, 2);
            ANALYSIS(constExp_2, ConstExp, 5);
            assert((TYPE_EQ_LITERAL(constExp_2)&&TYPE_EQ_LITERAL(constExp_1)) && "In frontend::Analyzer::analysisVarDef: Not a Literal");
            int val_1 = std::stoi(constExp_1->v);
            int val_2 = std::stoi(constExp_2->v);
            vector<int> dim = {val_1, val_2};
            int tot = val_1 * val_2;
            buffer.push_back(new Instruction(Operand(std::to_string(tot), Type::IntLiteral), Operand(), Operand(symbol_table.get_scoped_name(sVarName), tp), Operator::alloc));
            symbol_table.add_scope_entry(tp, sVarName, dim);
            return;
        }
        else{
            assert("In frontend::Analyzer::analysisVarDef: Unexpected Children Size");
        }
    }
}

// (computable = False, value, type): InitVal -> Exp | '{' [ InitVal { ',' InitVal } ] '}'
void frontend::Analyzer::analysisInitVal(InitVal* root, vector<ir::Instruction*>& buffer){
    // 看父节点类型，是VarDef再考虑是否赋值，否则只管求值即可?Alloc如何看？
    // 逻辑：父节点是数组则负责赋值，否则节点赋值
    // 查了下用例，{}分支只可能是数组初始赋值，但是{{},{}}要小心考虑
    AstNode *fa = root->parent;
    if (fa->type == NodeType::VARDEF){
        VarDef* parent = dynamic_cast<VarDef*>(fa);
        std::string arrName = parent->arr_name;
        STE arrSte = symbol_table.get_ste(arrName);
        Type tp = arrSte.operand.type;
        std::string arrScopedName = arrSte.operand.name;
        if (tp == Type::IntPtr || tp == Type::FloatPtr){        // 数组赋值，根据SysY文档，不可能出现{{1,2},{3,4}}的情况
            vector<int> dim = symbol_table.get_ste(arrName).dimension;
            int sz = 1;
            for (int i: dim){
                sz *= i;
            }
            if (root->children.size() == 2){
                for (int i = 0; i < sz;i++){
                    buffer.push_back(new Instruction(Operand(arrScopedName, tp), Operand(std::to_string(i), Type::IntLiteral), Operand("0", Type::IntLiteral), Operator::store));
                }
            }
            else{
                int childSize = root->children.size();
                for (int i = 1; i < childSize; i += 2){
                    ANALYSIS(constInitVal, ConstInitVal, i);
                    assert(TYPE_EQ_LITERAL(constInitVal) && "In frontend::Analyzer::analysisConstInitVal: Not a Literal in initializing array");
                    Type targetedTp = tp == Type::IntPtr ? Type::Int : Type::Float;
                    buffer.push_back(new Instruction(Operand(arrScopedName, tp), Operand(std::to_string(i / 2), Type::IntLiteral), Operand(constInitVal->v, targetedTp), Operator::store));
                }
            }
            return;
        }
    }
    // 只赋值
    ANALYSIS(exp, Exp, 0);
    root->v = exp->v;
    root->t = exp->t;
    return;
}

// (computable = False, value, type): Exp -> AddExp                                           // 加法表达式
void frontend::Analyzer::analysisExp(Exp* root, vector<ir::Instruction*>& buffer){
    ANALYSIS(addExpNode, AddExp, 0)
    COPY_EXP_NODE(addExpNode, root)
}

// FuncType -> 'void' | 'int' | 'float'
Token frontend::Analyzer::analysisFuncType(FuncType* root){
    GET_CHILD_PTR(term, Term, 0);
    return term->token;
}

// FuncFParam -> BType Ident ['[' ']' { '[' Exp ']' }]
Operand frontend::Analyzer::analysisFuncFParam(FuncFParam* root){
    GET_CHILD_PTR(btype, BType, 0);
    Token tk_type = analysisBType(btype);
    GET_CHILD_PTR(ident, Term, 1);
    Token tk_ident = ident->token;      // 应该是int, intPtr, float, floatPtr中的一个
    Type opType = root->children.size() == 2 ? (tk_type.type == TokenType::INTTK) ? Type::Int : Type::Float : 
                                               (tk_type.type == TokenType::INTTK) ? Type::IntPtr : Type::FloatPtr;
    Operand ret(tk_ident.value, opType);
    return ret;
}

// FuncFParams -> FuncFParam { ',' FuncFParam }            // 逗号表示重复
vector<Operand> frontend::Analyzer::analysisFuncFParams(FuncFParams* root){
    vector<Operand> ans;
    for (int i = 0; i < root->children.size(); i += 2){
        FuncFParam *son = dynamic_cast<FuncFParam*>(root->children[i]);
        Operand op = analysisFuncFParam(son);
        ans.push_back(op);
    }
    return ans;
}

// Block -> '{' { BlockItem } '}'                          // block由多个item构成，每个item以分号结尾，用大括号分隔表示block
void frontend::Analyzer::analysisBlock(Block* root, vector<ir::Instruction*>& buffer){
    for (int i = 1; i < root->children.size() - 1; i++){
        ANALYSIS(blockItem, BlockItem, i);
    }
}

// BlockItem -> Stmt | Decl                                // 每个Item由Statement或者声明构成
void frontend::Analyzer::analysisBlockItem(BlockItem* root, vector<ir::Instruction*>& buffer){
    GET_CHILD_PTR(son, AstNode, 0);
    if (son->type == NodeType::STMT){
        ANALYSIS(stmt, Stmt, 0);
    }
    else{
        ANALYSIS(decl, Decl, 0);
    }
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

// (computable = False, value, type, arrayIndex): LVal -> Ident {'[' Exp ']'}                             // 左值，可能是单个值也可能是数组,这个函数恶心在要返回指针可能
void frontend::Analyzer::analysisLVal(LVal* root, vector<ir::Instruction*>& buffer){
    const std::string V_PARSE_LVAL_PTR = "$lv";
    GET_CHILD_PTR(term, Term, 0);
    Token tk = term->token;
    Operand arrOperand = symbol_table.get_operand(tk.value);
    root->v = arrOperand.name;
    root->t = arrOperand.type;
    // a variable of Int or Float
    // TODO: 测试样例只有二维数组，那么就不弄复杂了，本来是应该算出来的
    // 草 文档给出的小坑：
    // int a[3][3]; int b = a[1];
    const std::string targetName = V_PARSE_LVAL_PTR + root->v;
    if (root->children.size() == 4){   // 一维数组，暂时都返回指针
        ANALYSIS(expNode1, Exp, 2);
        const vector<int>& dim = symbol_table.get_ste(tk.value).dimension;
        if (dim.size() == 1){
            buffer.push_back(new Instruction(arrOperand, Operand(expNode1->v, expNode1->t), Operand(targetName, root->t), Operator::getptr));
        } else {
            uint dim_2 = dim[1];
            buffer.push_back(new Instruction(Operand(std::to_string(dim_2), Type::IntLiteral), Operand(expNode1->v, expNode1->t), Operand("$arrOffset", Type::Int), Operator::mul));
            buffer.push_back(new Instruction(arrOperand, Operand("$arrOffset", Type::Int), Operand(targetName, root->t), Operator::getptr));
        }
        root->v = targetName;            // 记住哪一步要用这个了，就用$v_LVal变量计算就行
    }
    else if (root->children.size()!=1){       // 二维数组，暂时都返回指针
        ANALYSIS(expNode1, Exp, 2);
        ANALYSIS(expNode2, Exp, 5);
        int dim_2 = symbol_table.get_ste(tk.value).dimension[1];
        buffer.push_back(new Instruction(Operand(std::to_string(dim_2), Type::IntLiteral), Operand(expNode1->v, expNode1->t), Operand("$arrOffset", Type::Int), Operator::mul));
        buffer.push_back(new Instruction(Operand("$arrOffset", Type::Int), Operand(expNode2->v, expNode2->t), Operand("$arrOffset", Type::Int), Operator::add));
        buffer.push_back(new Instruction(arrOperand, Operand("$arrOffset", Type::Int), Operand(targetName, root->t), Operator::getptr));
        root->v = targetName;            // 记住哪一步要用这个了，就用$v_LVal变量计算就行
    }
}

// (computable = False, value, type): AddExp -> MulExp { ('+' | '-') MulExp }
void frontend::Analyzer::analysisAddExp(AddExp* root, vector<ir::Instruction*>& buffer){
    const std::string ADDEXP_CUMVARNAME = "$ad";
    int index = 0;
    ANALYSIS(mulExpNode_0, MulExp, index);
    COPY_EXP_NODE(mulExpNode_0, root);
    // 如果是个算术，那么$mu变量起累积作用，$mu为float或int
    // 累计变量处理，同时赋值
    if (root->children.size() != 1){        // 在这里可以把ptr处理了，变为int/float，处理成变量开始累积
        root->v = ADDEXP_CUMVARNAME;
        if (TYPE_EQ_PTR(root)){             // 指针
            root->t = TYPE_EQ(root, Type::FloatPtr) ? Type::Float : Type::Int;
            buffer.push_back(new Instruction(OPERAND_NODE(mulExpNode_0), Operand("0", Type::IntLiteral), OPERAND_NODE(root), Operator::load));
        }
        else if (TYPE_EQ_LITERAL(root)){    // 字面量
            root->t = TYPE_EQ(root, Type::FloatLiteral) ? Type::Float : Type::Int;
            if (TYPE_EQ(root, Type::Int)){
                buffer.push_back(new Instruction(OPERAND_NODE(mulExpNode_0), Operand(), OPERAND_NODE(root), Operator::def));
            }
            else{
                buffer.push_back(new Instruction(OPERAND_NODE(mulExpNode_0), Operand(), OPERAND_NODE(root), Operator::fdef));
            }
        }
        else{                               // 变量
            buffer.push_back(new Instruction(OPERAND_NODE(mulExpNode_0), Operand(), OPERAND_NODE(root), Operator::mov));
        }
    }
    // 开始计算
    while (index + 1 < root->children.size())
    {
        GET_CHILD_PTR(term, Term, ++index);
        Token tk = term->token;
        ANALYSIS(mulExpNode, MulExp, ++index);
        // 累计变量的类型转换
        if (TYPE_EQ_INT(root) && TYPE_EQ_F_LTR_PTR(mulExpNode))
        {
            root->t = Type::Float;
            buffer.push_back(new Instruction(Operand(root->v, Type::Int), Operand(), Operand(root->v, Type::Float), Operator::cvt_i2f));
        }
        switch (tk.type)
        {
        case TokenType::PLUS:
            cumulativeComputing(OPERAND_NODE(root), OPERAND_NODE(mulExpNode), Operator::add, buffer);
            break;
        case TokenType::MINU:
            cumulativeComputing(OPERAND_NODE(root), OPERAND_NODE(mulExpNode), Operator::sub, buffer);
            break;
        default:
            assert(0 && "In analysisAddExp: Unexpected Input!");
            break;
        }
    }
}

// (computable = False, value, type): Cond -> LOrExp                                          
void frontend::Analyzer::analysisCond(Cond* root, vector<ir::Instruction*>& buffer){
    ANALYSIS(lOrExp, LOrExp, 0);
    COPY_EXP_NODE(lOrExp, root);
}

// (computable = False, value, type = int): LOrExp -> LAndExp [ '||' LOrExp ]
void frontend::Analyzer::analysisLOrExp(LOrExp* root, vector<ir::Instruction*>& buffer){
    const std::string tem = "$lor";
    ANALYSIS(lAndExp, LAndExp, 0);
    COPY_EXP_NODE(lAndExp, root);
    if (root->children.size() == 3){
        ANALYSIS(lOrExp, LOrExp, 2);
        Operand des(tem, Type::Int);
        buffer.push_back(new Instruction(OPERAND_NODE(root), OPERAND_NODE(lOrExp), des, Operator::_or));
        root->v = des.name;
        root->t = des.type;
    }
}

// (computable = True, value, type): Number -> floatConst | IntConst 
void frontend::Analyzer::analysisNumber(Number* root, vector<ir::Instruction*>& buffer){
    GET_CHILD_PTR(term, Term, 0);
    Token tk = term->token;
    root->v = tk.value;
    root->is_computable = true;
    root->t = tk.type == TokenType::FLOATLTR ? Type::FloatLiteral : Type::IntLiteral;
}

// (computable = False, value, type): PrimaryExp -> '(' Exp ')' | LVal | Number               // 右边表达式的单个项的可能值，可以是括号，数字和变量
// 注意这个节点上去，六种情况(null, ptr, literal) * (Int, Float)都他妈有可能出现，一直到Exp，都要充分考虑到六种情况！！！
void frontend::Analyzer::analysisPrimaryExp(PrimaryExp* root, vector<ir::Instruction*>& buffer){
    if (root->children.size() == 3){    // ( Exp )
        ANALYSIS(expNode, Exp, 1);
        COPY_EXP_NODE(expNode, root);
    }
    else if (root->children[0]->type == NodeType::NUMBER){  // Number Literal两种
        ANALYSIS(numberNode, Number, 0);
        COPY_EXP_NODE(numberNode, root);
    }
    else{       // LVAL 四种情况返回
        ANALYSIS(lvalNode, LVal, 0);
        COPY_EXP_NODE(lvalNode, root);
    }
}

// (computable = False, value, type):  UnaryExp -> PrimaryExp                                  // 单个项
//                                     UnaryExp -> Ident '(' [FuncRParams] ')'                 // 函数
//                                     UnaryExp -> UnaryOp UnaryExp   
// 返回的仍然是六种情况，2，3行只有两种情况
void frontend::Analyzer::analysisUnaryExp(UnaryExp* root, vector<ir::Instruction*>& buffer){
    const std::string V_UNARYEXP_VARNAME = "$uae";
    if (root->children.size() == 1){    // Line 1
        ANALYSIS(primaryExp, PrimaryExp, 0);
        COPY_EXP_NODE(primaryExp, root);
    } 
    else if (root->children.size() == 2){     // Line 3
        /* 这里就值得好好考虑了，六种情况，v和t该怎么处理？
         * 值得注意的是，这里返回只有四种情况了
         * 显然PrimaryExp传指针上去便于FuncRParam去getptr(虽然但是，我在LVal里面的解析中就getptr了，
         * 情况不对（被当作Exp的一部分运算后）立马load出来，以及Stmt会对LVal进行赋值，这样做其实还蛮好的)
         * 现在来看看可能的六种情况，关注root的v和t怎么变：
         * Int:     
         * Float:           最典型了，后面特别注意类型转换
         * IntLiteral:
         * FloatLiteral:    emm，我选择直接观察v值，改一下传上去就行
         * IntPtr:
         * FloatPtr:        啊哦，这里就可以load出来了
         */
        ANALYSIS(unaryExpNode, UnaryExp, 1);
        GET_CHILD_PTR(unaryOpNode, UnaryOp, 0);
        Token tk = analysisUnaryOp(unaryOpNode, buffer);
        root->is_computable = unaryExpNode->is_computable;
        std::string targetVar = V_UNARYEXP_VARNAME + root->v;
        if (TYPE_EQ_PTR(unaryExpNode)){ // 指针Load出来
            root->v = targetVar;
            root->t = unaryExpNode->t == Type::IntPtr ? Type::Int : Type::Float;
            buffer.push_back(new Instruction(Operand(unaryExpNode->v, unaryExpNode->t), Operand("0", Type::IntLiteral), Operand(root->v, root->t), Operator::load));
        }
        switch (tk.type){
            case TokenType::PLUS:
                break;
            case TokenType::MINU:
                if (root->t == Type::Int){
                    buffer.push_back(new Instruction(Operand("0", Type::IntLiteral), Operand(), Operand("$tem0", Type::Int), Operator::def));
                    buffer.push_back(new Instruction(Operand("$tem0", Type::Int), Operand(root->v, root->t), Operand(targetVar, root->t), Operator::sub));
                } else if (root->t == Type::Float) {
                    buffer.push_back(new Instruction(Operand("0.0", Type::FloatLiteral), Operand(), Operand("$ftem0", Type::Float), Operator::fdef));
                    buffer.push_back(new Instruction(Operand("$ftem0", Type::Float), Operand(root->v, root->t), Operand(targetVar, root->t), Operator::fsub));
                } else if (TYPE_EQ_LITERAL(root)) {
                    root->v = root->v[0] == '-' ? root->v.substr(1) : "-" + root->v;
                } else {
                    assert(0 && "In analysisUnaryExp: Unexpected Type");
                }
                break;
            case TokenType::NOT:
                // TODO BUG: 看文档只接受int，那我姑且认为进来来算的一定是int咯，有float报错了再回来cvt一下hhh，先懒得写了，考虑到底层表示，浮点数和整数都是全0，（一个例外是浮点数的负零）。我堵他没有这个特例，不考虑了
                buffer.push_back(new Instruction(Operand(unaryExpNode->v, Type::Int), Operand(), Operand(targetVar, Type::Int), Operator::_not));
                break;
            default:
                break;
        }
        if (TYPE_EQ_LITERAL(root)){
            root->v = targetVar;
        }
    } 
    // UnaryExp -> Ident '(' [FuncRParams] ')'                 // 函数
    else {    // Line 2
        assert("In UnaryExp: TODO: hidden cast for paramList");
        // 函数的Ident我们放在symbol_table[0]下面
        GET_CHILD_PTR(term, Term, 0);
        Token tk = term->token;
        std::string funcName = tk.value;
        if (root->children.size() == 4){    // FuncParams
            GET_CHILD_PTR(funcRParams, FuncRParams, 2);
            vector<Operand> paramList = analysisFuncRParams(funcRParams, buffer);
            Type retType = symbol_table.get_operand_type(funcName, true);
            ir::CallInst* callInst = new ir::CallInst(Operand(funcName, retType), paramList, Operand("$ret", retType));
            buffer.push_back(callInst);
        }
        else{
            Type retType = symbol_table.get_operand_type(funcName, true);
            ir::CallInst* callInst = new ir::CallInst(Operand(funcName, retType), Operand("$ret", retType));
            buffer.push_back(callInst);
        }
        // Instruction callInst = ir::CallInst(函数返回值, 形参列表, Operand());
        // buffer.push_back(&callInst);
    }
}

// (TokenType op): UnaryOp -> '+' | '-' | '!'
Token frontend::Analyzer::analysisUnaryOp(UnaryOp* root, vector<ir::Instruction*>& buffer){
    GET_CHILD_PTR(son, Term, 0);
    return son->token;
}

// FuncRParams -> Exp { ',' Exp }                          // 调用函数时的参数
vector<Operand> frontend::Analyzer::analysisFuncRParams(FuncRParams* root, vector<ir::Instruction*>& buffer){
    vector<Operand> ans;
    uint index = 0;
    while(index <= root->children.size() - 1){
        ANALYSIS(exp, Exp, index);
        Operand opd(exp->v, exp->t);
        ans.push_back(opd);
        index += 2;
    }
    return ans;
}

// (computable = False, value, type): MulExp -> UnaryExp { ('*' | '/' | '%') UnaryExp }        
// 注意返回上来的依旧是六个情况             
void frontend::Analyzer::analysisMulExp(MulExp* root, vector<ir::Instruction*>& buffer){
    const std::string MULEXP_CUMVARNAME = "$muc";
    int index = 0;
    ANALYSIS(unaryExpNode_0, UnaryExp, index);
    COPY_EXP_NODE(unaryExpNode_0, root);
    // 如果是个算术，那么$mu变量起累积作用，$mu为float或int
    // 不为1则开始累积
    if (root->children.size() != 1){        // 在这里可以把ptr处理了，变为int/float，处理成变量开始累积
        root->v = MULEXP_CUMVARNAME;
        if (TYPE_EQ_PTR(root)){             // 指针
            root->t = TYPE_EQ(root, Type::FloatPtr) ? Type::Float : Type::Int;
            buffer.push_back(new Instruction(OPERAND_NODE(unaryExpNode_0), Operand("0", Type::IntLiteral), OPERAND_NODE(root), Operator::load));
        }
        else if (TYPE_EQ_LITERAL(root)){    // 字面量
            root->t = TYPE_EQ(root, Type::FloatLiteral) ? Type::Float : Type::Int;
            if (TYPE_EQ(root, Type::Int)){
                buffer.push_back(new Instruction(OPERAND_NODE(unaryExpNode_0), Operand(), OPERAND_NODE(root), Operator::def));
            }
            else{
                buffer.push_back(new Instruction(OPERAND_NODE(unaryExpNode_0), Operand(), OPERAND_NODE(root), Operator::fdef));
            }
        }
        else{                               // 变量
            buffer.push_back(new Instruction(OPERAND_NODE(unaryExpNode_0), Operand(), OPERAND_NODE(root), Operator::mov));
        }
    }
    // 开始计算
    while (index + 1 < root->children.size())
    {
        GET_CHILD_PTR(term, Term, ++index);
        Token tk = term->token;
        ANALYSIS(unaryExpNode, UnaryExp, ++index);
        // 累计变量的类型转换
        if (TYPE_EQ_INT(root) && TYPE_EQ_F_LTR_PTR(unaryExpNode))
        {
            root->t = Type::Float;
            buffer.push_back(new Instruction(Operand(root->v, Type::Int), Operand(), Operand(root->v, Type::Float), Operator::cvt_i2f));
        }
        switch (tk.type)
        {
        case TokenType::MULT:
            cumulativeComputing(OPERAND_NODE(root), OPERAND_NODE(unaryExpNode), Operator::mul, buffer);
            break;
        case TokenType::DIV:
            cumulativeComputing(OPERAND_NODE(root), OPERAND_NODE(unaryExpNode), Operator::div, buffer);
            break;
        case TokenType::MOD:           // 说是整型变量取余，那就不考虑了，报错了算我倒霉
            buffer.push_back(new Instruction(root->v, unaryExpNode->v, root->v, Operator::mod));
            break;
        default:
            assert(0 && "In analysisMulExp: Unexpected Input!");
        }
    }
}

// (computable = False, value, type = int): RelExp -> AddExp { ('<' | '>' | '>=' | '<=') AddExp }
void frontend::Analyzer::analysisRelExp(RelExp* root, vector<ir::Instruction*>& buffer){
    const std::string tem = "$rel";
    ANALYSIS(addExp_0, AddExp, 0);
    COPY_EXP_NODE(addExp_0, root);
    if (root->children.size() == 1)
        return;
    // 处理多次相等的情况，注意左结合性
    uint index = 2;
    Operand des(tem, Type::Int);
    ANALYSIS(addExp, AddExp, index);
    GET_CHILD_PTR(term, Term, index - 1);
    Operator opt;
    switch (term->token.type)
    {
    case TokenType::LEQ:
        opt = Operator::leq;
        break;
    case TokenType::LSS:
        opt = Operator::lss;
        break;
    case TokenType::GEQ:
        opt = Operator::geq;
        break;
    case TokenType::GTR:
        opt = Operator::gtr;
        break;
    default:
        assert(0 && "In frontend::Analyzer::analysisRelExp: Invalid operator type");
        break;
    }
    buffer.push_back(new Instruction(OPERAND_NODE(root), OPERAND_NODE(addExp), des, opt));
    index += 2;
    while (index <= root->children.size() - 1){
        ANALYSIS(addExp, AddExp, index);
        GET_CHILD_PTR(term, Term, index - 1);
        Operator opt;
        switch (term->token.type)
        {
        case TokenType::LEQ:
            opt = Operator::leq;
            break;
        case TokenType::LSS:
            opt = Operator::lss;
            break;
        case TokenType::GEQ:
            opt = Operator::geq;
            break;
        case TokenType::GTR:
            opt = Operator::gtr;
            break;
        default:
            assert(0 && "In frontend::Analyzer::analysisRelExp: Invalid operator type");
            break;
        }
        buffer.push_back(new Instruction(des, OPERAND_NODE(addExp), des, opt));
        index += 2;
    }
    root->v = des.name;
    root->t = des.type;
}

// (computable = False, value, type = int): EqExp -> RelExp { ('==' | '!=') RelExp }
void frontend::Analyzer::analysisEqExp(EqExp* root, vector<ir::Instruction*>& buffer){
    const std::string tem = "$eq";
    ANALYSIS(relExp_0, RelExp, 0);
    COPY_EXP_NODE(relExp_0, root);
    if (root->children.size() == 1)
        return;
    // 处理多次相等的情况，注意左结合性
    uint index = 2;
    Operand des(tem, Type::Int);
    ANALYSIS(relExp, RelExp, index);
    GET_CHILD_PTR(term, Term, index - 1);
    Operator opt;
    switch (term->token.type)
    {
    case TokenType::NEQ:
        opt = Operator::neq;
        break;
    case TokenType::EQL:
        opt = Operator::eq;
        break;
    default:
        assert(0 && "In frontend::Analyzer::analysisEqExp: Invalid operator type");
        break;
    }
    buffer.push_back(new Instruction(OPERAND_NODE(root), OPERAND_NODE(relExp), des, opt));
    index += 2;
    while (index <= root->children.size() - 1){
        ANALYSIS(relExp, RelExp, index);
        GET_CHILD_PTR(term, Term, index - 1);
        Operator opt;
        switch (term->token.type)
        {
        case TokenType::NEQ:
            opt = Operator::neq;
            break;
        case TokenType::EQL:
            opt = Operator::eq;
            break;
        default:
            assert(0 && "In frontend::Analyzer::analysisEqExp: Invalid operator type");
            break;
        }
        buffer.push_back(new Instruction(des, OPERAND_NODE(relExp), des, opt));
        index += 2;
    }
    root->v = des.name;
    root->t = des.type;
}

// (computable = False, value, type = int): LAndExp -> EqExp [ '&&' LAndExp ]
void frontend::Analyzer::analysisLAndExp(LAndExp* root, vector<ir::Instruction*>& buffer){
    const std::string tem = "$and";
    ANALYSIS(lEqExp, EqExp, 0);
    COPY_EXP_NODE(lEqExp, root);
    if (root->children.size() == 3){
        ANALYSIS(lAndExp, LAndExp, 2);
        Operand des(tem, Type::Int);
        buffer.push_back(new Instruction(OPERAND_NODE(root), OPERAND_NODE(lAndExp), des, Operator::_and));
        root->v = des.name;
        root->t = des.type;
    }
}

// 我们在这里做如下假设：
// cumVar必定是Int或Float
// cumVar如果是Int，则UpVar必定是Int, IntLiteral和IntPtr中的一个
// 这个函数主要处理相当恶心的类型转换，而且尽量减少IR的数量
// 职责：根据cumVar的类型，对upVar做类型转换（float，int以及各种literal和ptr），并添加相应IR指令
void frontend::Analyzer::cumulativeComputing(Operand cumVar, Operand upVar, Operator opt, vector<ir::Instruction*>& buffer){
#define TEM_PTR_VAR "$P2V"
    // 处理opt
    switch (opt)
    {
    // fxx, xx, xxi
    // xx不接受f，f只接受float, xxi第二个必须是i，不接受
    case Operator::add:
    case Operator::addi:
    case Operator::fadd:
        switch (upVar.type)
        {
        case ir::Type::IntLiteral:
            opt = Operator::addi;           // 为什么有这种。。
            break;
        case ir::Type::FloatLiteral:
        case ir::Type::Float:
            opt = Operator::fadd;
            break;
        default:
            opt = Operator::add;
            break;
        }
        break;
    case Operator::sub:
    case Operator::subi:
    case Operator::fsub:
        switch (upVar.type)
        {
        case ir::Type::IntLiteral:
            opt = Operator::subi;
            break;
        case ir::Type::FloatLiteral:
        case ir::Type::Float:
            opt = Operator::fsub;
            break;
        default:
            opt = Operator::sub;
            break;
        }
        break;
    case Operator::mul:
    case Operator::fmul:
        switch (upVar.type)
        {
        case ir::Type::FloatLiteral:
        case ir::Type::Float:
            opt = Operator::fmul;
            break;
        default:
            opt = Operator::mul;
            break;
        }
        break;
    case Operator::div:
    case Operator::fdiv:
        switch (upVar.type)
        {
        case ir::Type::FloatLiteral:
        case ir::Type::Float:
            opt = Operator::fdiv;
            break;
        default:
            opt = Operator::div;
            break;
        }
        break;
    default:
        assert(0 && "In cumulative Computing: Unexpected opt!");
        break;
    }
    // 处理要来的变量，主要是指针
    bool isPtr = false;
    Operand tempPtrVar;
    if (upVar.type == Type::IntPtr){
        isPtr = true;
        tempPtrVar.name = TEM_PTR_VAR;
        tempPtrVar.type = Type::Int;
        buffer.push_back(new Instruction(cumVar, Operand(), tempPtrVar, Operator::def));
    }
    else if (upVar.type == Type::FloatPtr){
        isPtr = true;
        tempPtrVar.name = TEM_PTR_VAR;
        tempPtrVar.type = Type::Float;
        buffer.push_back(new Instruction(cumVar, Operand(), tempPtrVar, Operator::fdef));
    }
    Operand finalOpt;
    finalOpt.name = isPtr ? tempPtrVar.name : upVar.name;
    finalOpt.type = isPtr ? tempPtrVar.type : upVar.type;
    buffer.push_back(new Instruction(cumVar, finalOpt, cumVar, opt));
#undef TEM_F_VAR
}

// 搞了半天，发现这个函数十分有必要：将某个Operand转换为期望类型
// 其中期望类型必定不是指针
// 反正就6*4=24种情况
// 返回期望的Operand
// 本函数可能发生的变化： Literal->Var, Ptr->Var, Var->Var, type_cast
// 涉及到类型转换就一定有Operand出现，指Literal->xx, 不可能有Literal->Literal
// 步骤1： 平凡情况判等，return
// 步骤2：判断字面量的类型转换，return
// 步骤3：判断指针，读取出来，不return，operand变量变成var，注意ir指令压的啥
// 步骤4：判断var的类型转换
Operand frontend::Analyzer::castExpectedType(Operand operand, Type tp, vector<ir::Instruction*>& buffer){
    assert((operand.type != Type::null) && "In frontend::Analyzer::castExpectedType: null operand!!!");
    assert((Type::Int == tp || Type::Float == tp) && "In frontend::Analyzer::castExpectedType: unexpected target type!!!");
    if (operand.type == tp)
        return operand;
    const std::string prefix = "$cst";
    std::string srcVarName = operand.name;
    std::string srcTarName = prefix + operand.name;
    if (operand.type == Type::IntLiteral){  // IntLiteral -> Int | Float
        Operand retOpe(srcTarName, tp);
        if (tp == Type::Float){
            buffer.push_back(new Instruction(operand, Operand(), retOpe, Operator::cvt_i2f));
        }
        else{
            buffer.push_back(new Instruction(operand, Operand(), retOpe, Operator::def));
        }
        return retOpe;
    }
    else if(operand.type == Type::FloatLiteral){    // FloatLiteral -> Int | Float
        Operand retOpe(srcTarName, tp);
        if (tp == Type::Int){
            buffer.push_back(new Instruction(operand, Operand(), retOpe, Operator::cvt_f2i));
        }
        else{
            buffer.push_back(new Instruction(operand, Operand(), retOpe, Operator::fdef));
        }
        return retOpe;

    }
    // load ptr
    if (operand.type == Type::IntPtr){      // IntPtr -> Int
        Operand targetOperand(srcTarName, Type::Int);
        Operand base("0", Type::Int);
        buffer.push_back(new Instruction(operand, base, targetOperand, Operator::load));
        operand.name = srcTarName;
        operand.type = Type::Int;
    }
    else if (operand.type == Type::FloatPtr){   // FloatPtr -> Float
        Operand targetOperand(srcTarName, Type::Float);
        Operand base("0", Type::Int);
        buffer.push_back(new Instruction(operand, base, targetOperand, Operator::load));
        operand.name = srcTarName;
        operand.type = Type::Float;
    }
    // consider convert Var->Var
    if (operand.type == tp){        // FloatPtr->Float == Float, Int is the same, namely equal type
        return operand;
    }
    operand.type = tp;
    if (tp == Type::Int){
        buffer.push_back(new Instruction(Operand(operand.name, Type::Float), Operand(), operand, Operator::cvt_f2i));
    }
    else{
        buffer.push_back(new Instruction(Operand(operand.name, Type::Int), Operand(), operand, Operator::cvt_i2f));
    }
    return operand;
}
