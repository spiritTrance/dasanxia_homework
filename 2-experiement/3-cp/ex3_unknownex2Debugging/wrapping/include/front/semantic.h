/**
 * @file semantic.h
 * @author Yuntao Dai (d1581209858@live.com)
 * @brief 
 * @version 0.1
 * @date 2023-01-06
 * 
 * a Analyzer should 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef SEMANTIC_H
#define SEMANTIC_H

#include"ir/ir.h"
#include"front/abstract_syntax_tree.h"

#include<map>
#include<string>
#include<vector>
using std::map;
using std::string;
using std::vector;

namespace frontend
{

// definition of symbol table entry
struct STE {
    ir::Operand operand;
    vector<int> dimension;
};

using map_str_ste = map<string, STE>;
// definition of scope infomation
struct ScopeInfo {
    int cnt;    // 作用域在函数中的唯一编号, 代表是函数中出现的第几个作用域
    string name;    // 分辨作用域的类别, 'b' 代表是一个单独嵌套的作用域, 'i' 'e' 'w' 分别代表由 if else while 产生的新作用域
    map_str_ste table;      //string 是操作数的【原始】名称, 在 STE 中存放的应该是【变量重命名】后的名称
};

// surpport lib functions
map<std::string,ir::Function*>* get_lib_funcs();

// definition of symbol table
struct SymbolTable{
    vector<ScopeInfo> scope_stack;      // idents record, sort by scope
    map<std::string,ir::Function*> functions;   // record functions
    int count = 0;

    /**
     * @brief enter a new scope, record the infomation in scope stacks
     * @param s: entering a new Scope means a new name scope
     */
    void add_scope(std::string);

    /**
     * @brief add a new entry at the last scope
     * @param ste: a ste entry
     */
    void add_scope_entry(frontend::STE, bool);
    void add_scope_entry(ir::Operand, std::vector<int>, bool);
    void add_scope_entry(ir::Type, std::string, std::vector<int>, bool);
    void add_scope_const_entry(ir::Type, std::string, std::string);

    /**
     * @brief exit a scope, pop out infomations
     */
    void exit_scope();

    /**
     * @brief Get the scoped name, to deal the same name in different scopes, we change origin id to a new one with scope infomation,
     * for example, we have these code:
     * "     
     * int a;
     * {
     *      int a; ....
     * }
     * "
     * in this case, we have two variable both name 'a', after change they will be 'a' and 'a_block'
     * @param id: origin id 
     * @return string: new name with scope infomations
     */
    string get_scoped_name(string id) const;

    /**
     * @brief get the right operand with the input name
     * @param id identifier name
     * @return Operand 
     */
    ir::Operand get_operand(string id, bool) const;
    ir::Type get_operand_type(string id, bool) const;
    std::string get_operand_name(string id, bool) const;
    ir::Type getCurrFuncType() const;
    std::string getCurrScopeName() const;
    // unsigned int get_array_offset(std::string, vector<int>) const;

    /**
     * @brief get the right ste with the input name
     * @param id identifier name
     * @return STE 
     */
    STE get_ste(string id, bool) const;
};


// singleton class
struct Analyzer {
    int tmp_cnt;
    vector<ir::Instruction*> g_init_inst;           // 全局区域的inst
    SymbolTable symbol_table;

    /**
     * @brief constructor
     */
    Analyzer();

    // analysis functions
    ir::Program get_ir_program(CompUnit*);
    void analysisCompUnit(CompUnit*, ir::Program&);
    void analysisDecl(Decl*, vector<ir::Instruction*>&);
    void analysisFuncDef(FuncDef*, ir::Function*&);
    void analysisConstDecl(ConstDecl*, vector<ir::Instruction*>&);
    void analysisVarDecl(VarDecl*, vector<ir::Instruction*>&);
    frontend::Token analysisBType(BType*);
    void analysisConstDef(ConstDef*, vector<ir::Instruction*>&);
    void analysisConstInitVal(ConstInitVal*, vector<ir::Instruction*>&);
    void analysisConstExp(ConstExp*, vector<ir::Instruction*>&);
    void analysisVarDef(VarDef*, vector<ir::Instruction*>&);
    void analysisInitVal(InitVal*, vector<ir::Instruction*>&);
    void analysisExp(Exp*, vector<ir::Instruction*>&);
    frontend::Token analysisFuncType(FuncType*);
    ir::Operand analysisFuncFParam(FuncFParam*, vector<ir::Instruction*>&);
    std::vector<ir::Operand> analysisFuncFParams(FuncFParams*, vector<ir::Instruction*>&);
    void analysisBlock(Block*, vector<ir::Instruction*>&);
    void analysisBlockItem(BlockItem*, vector<ir::Instruction*>&);
    void analysisStmt(Stmt*, vector<ir::Instruction*>&);
    void analysisLVal(LVal*, vector<ir::Instruction*>&);
    void analysisAddExp(AddExp*, vector<ir::Instruction*>&);
    void analysisCond(Cond*, vector<ir::Instruction*>&);
    void analysisLOrExp(LOrExp*, vector<ir::Instruction*>&);
    void analysisNumber(Number*, vector<ir::Instruction*>&);
    void analysisPrimaryExp(PrimaryExp*, vector<ir::Instruction*>&);
    void analysisUnaryExp(UnaryExp*, vector<ir::Instruction*>&);
    frontend::Token analysisUnaryOp(UnaryOp*, vector<ir::Instruction*>&);
    std::vector<ir::Operand> analysisFuncRParams(FuncRParams*, vector<ir::Operand>& ,vector<ir::Instruction*>&);
    void analysisMulExp(MulExp*, vector<ir::Instruction*>&);
    void analysisRelExp(RelExp*, vector<ir::Instruction*>&);
    void analysisEqExp(EqExp*, vector<ir::Instruction*>&);
    void analysisLAndExp(LAndExp*, vector<ir::Instruction*>&);
    void cumulativeComputing(ir::Operand, ir::Operand, ir::Operator, vector<ir::Instruction*>&);
    ir::Operand castExpectedType(ir::Operand, ir::Type, vector<ir::Instruction*>&);
    bool constNumberComputing(ir::Operand, ir::Operand, ir::Operand&, frontend::TokenType);
    std::string getReturnTempName(vector<ir::Operand>&) const;
    // reject copy & assignment
    Analyzer(const Analyzer&) = delete;
    Analyzer& operator=(const Analyzer&) = delete;
};

} // namespace frontend

#endif