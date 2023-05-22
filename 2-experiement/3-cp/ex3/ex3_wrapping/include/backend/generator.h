#ifndef GENERARATOR_H
#define GENERARATOR_H

#include "ir/ir.h"
#include "backend/rv_def.h"
#include "backend/rv_inst_impl.h"

#include<map>
#include<string>
#include<vector>
#include<queue>
#include<stack>
#include<fstream>

namespace backend {

// it is a map bewteen variable and its mem addr, 
// the mem addr of a local variable can be identified by 【($sp + off)】
struct stackVarMap {
    // 注意存的是与sp的相对位置（改成fp）
    // 想一下如何维护？手中有sp和fp，建议是offset参照fp来，更好确定
    std::map<ir::Operand, int> stack_table;
    // 调用者要保存寄存器的个数
    /**
     * @brief find the addr of a ir::Operand
     * @return the offset
    */
    int find_operand(ir::Operand);

    /**
     * @brief add a ir::Operand into current map, alloc space for this variable in memory 
     * @param[in] size: the space needed(in byte)
     * @return the offset
    */
    int add_operand(ir::Operand, uint32_t size = 4);
};

struct Generator {
    const ir::Program& program;         // the program to gen
    std::ofstream& fout;                 // output file
    std::vector<stackVarMap> memvar_Stack;  // 注意是栈式存储，这里我们假设sp总为0，且sp总为调整过后的。
    Generator(ir::Program&, std::ofstream&);
    // 记录寄存器到Operand之间的双射，全局变量的Operand也在此列
    std::map<ir::Operand, rv::rvFREG> f_opd2regTable;
    std::map<rv::rvFREG, ir::Operand> f_reg2opdTable;
    std::map<ir::Operand, rv::rvREG> i_opd2regTable;
    std::map<rv::rvREG, ir::Operand> i_reg2opdTable;
    // const parameters
    // reg flag
    
    unsigned int i_validReg;         // 标识存有数据的寄存器
    unsigned int f_validReg;         // 标识存有数据的寄存器
    unsigned int i_imAtomicComp;     // 标识当前指令要参与计算的寄存器
    unsigned int f_imAtomicComp;     // 标识当前指令要参与计算的寄存器
    // 负责生成flag要维护的相关信息，记得每进行一个函数记得清空。
    unsigned int ir_stamp = 0;      // 每进入一个函数就清空，第一条指令为1不是0
    std::queue<int> flag_q;         // 由于指令是顺序扫描的，因此元素记录了该跳进哪个ir
    unsigned int index_flag[100000] = {};       // 记录第几个ir应当打印第几个flag，e.g. index_flag[4] = 3, 则应该在第四条ir指令开头打印.L3
    unsigned int goto_flag = 1;        // 从1开始，从不恢复
    // reg allocate api
    bool isNewOperand(ir::Operand);
    bool isInReg(ir::Operand);
    bool isInStack(ir::Operand);
    bool isGlobalVar(ir::Operand);
    void expireRegData(int, int);       // 将寄存器里面的值移回到内存，注意可能是全局变量
    void loadMemData(int, ir::Operand);         // 将内存里面的值读进到寄存器
    rv::rvREG getRd(ir::Operand);
    rv::rvREG getRs1(ir::Operand);
    rv::rvREG getRs2(ir::Operand);
    rv::rvFREG fgetRd(ir::Operand);
    rv::rvFREG fgetRs1(ir::Operand);
    rv::rvFREG fgetRs2(ir::Operand);

    // generate wrapper function
    void gen();
    void gen_func(ir::Function&);
    void gen_instr(ir::Instruction&);
    void gen_globalVal();   // 生成全局变量段的

    // 跳转相关的标签维护
    void get_ir_flagInfo(std::vector<ir::Instruction *>&);
    // 获取栈中的Operand
    // FIXME: 删除这两个函数，功能和find_operand重叠
    ir::Operand getOperandFromStackSpace(ir::Operand);
    int getOffSetFromStackSpace(ir::Operand);

    int find_operand(ir::Operand);
    int add_operand(ir::Operand, uint32_t size = 4);    // 栈空间里面分配空间

    // // 寄存器管理
    // 进入一个函数首先调用callee寄存器
    int calleeRegisterSave();   // 返回保存了多少个寄存器，要求在进入函数前调用
    int calleeRegisterRestore();   // 返回保存了多少个寄存器，要求在进入函数前调用
    // 要调用的时候调用caller寄存器
    int callerRegisterSave();
    int callerRegisterRestore();   // 返回保存了多少个寄存器，要求在进入函数前调用
    // void findOperandInRegFile(ir::Operand);

    int getStackSpaceSize(std::vector<ir::Instruction *> &);

    // algorithm FIXME TODO
    // std::vector<ir::Operand> linearScan(const std::vector<ir::Instruction *>) const;
};

struct GlobalValElement{
    // 考虑一下这个 initArr 怎么用，传进来的时候要把维度算出来。
    std::string sVarName;
    ir::Type tp;
    // 注意浮点数的情况
    std::vector<int32_t> initArr;
    int maxLen;

    GlobalValElement(){};
    GlobalValElement(std::string, ir::Type, std::vector<int32_t>, int);
    GlobalValElement operator=(const GlobalValElement&);
    GlobalValElement(const GlobalValElement&);
    void draw(std::ofstream&);
};

} // namespace backend


#endif