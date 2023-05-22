#include "backend/generator.h"
#include <iostream>
#include <queue>
#include <set>
#include <assert.h>
#include <string.h>
#define TODO assert(0 && "todo");
#define endl "\n"

/*
 *  在开始之前，我强调一下全局变量的处理：
 *      对于变量，全局变量的处理在(f)getR*函数中处理
 *      对于数组，涉及到的IR也就那几个，getptr, alloc, load, store这几个，看IR->RISCV的转换函数，有处理
 */

using namespace ir;
using std::cout;

backend::Generator::Generator(ir::Program& p, std::ofstream& f): program(p), fout(f) {}

int backend::stackVarMap::find_operand(ir::Operand op){
    assert(stack_table.find(op) != stack_table.end() && "No such a operand!");
    return stack_table[op];
}

int backend::stackVarMap::add_operand(ir::Operand op, uint32_t size){
    // 感觉可以全用fp算偏移，然后sp看怎么弄
    assert(size % 4 == 0 && "Size should be the multiple of 4");
    int currSize = stack_table.size() * 4;
    int totSize = currSize + size;
    stack_table[op] = totSize;
    return totSize;        // 相对于fp来算的话，fp是大地址，那么-fp是小地址，数组的话，正向偏移，和xx一致的。
}

bool backend::Generator::isNewOperand(ir::Operand op){
    return !isInReg(op) && !isInStack(op);
}

bool backend::Generator::isGlobalVar(ir::Operand op){
    const auto &vec = program.globalVal;
    for (const auto& i: vec){
        if (i.val.name == op.name){
            return true;
        }
    }
    return false;
}

bool backend::Generator::isInStack(ir::Operand op){
    int idx = memvar_Stack.size() - 1;
    assert(idx != -1 && "Unexpected memory variable size of 0!");
    auto it = memvar_Stack[idx].stack_table.find(op);
    return it != memvar_Stack[idx].stack_table.end();
}

bool backend::Generator::isInReg(ir::Operand op){
    if (op.type == Type::Float  || op.type == Type::FloatPtr || op.type == Type::FloatLiteral){
        auto it = f_opd2regTable.find(op);
        return it != f_opd2regTable.end();
    }
    else if (op.type == Type::IntPtr  || op.type == Type::Int || op.type == Type::IntLiteral){
        auto it = i_opd2regTable.find(op);
        return it != i_opd2regTable.end();
    }
    else{
        assert(0 && "Unexpected Type!");
    }
}

// caseOp = 0: int, caseOp = 1: float
void backend::Generator::expireRegData(int regIndex, int caseOp){
    if (caseOp == 0){   // 整数
        rv::rvREG reg = rv::rvREG(regIndex);
        Operand opd = i_reg2opdTable[reg];
        if (isGlobalVar(opd)){      // 全局变量，约定X7用来存储地址（包括浮点数也是）
            fout<<"\t"<<"lui\t"<<toGenerateString(rv::rvREG::X7)<<","<<"\%hi("<<opd.name<<")"<<endl;
            fout<<"\t"<<"sw\t"<<toGenerateString(reg)<<","<<"\%lo("<<opd.name<<")("<<toGenerateString(rv::rvREG::X7)<<")"<<endl;
        }
        else{
            // 写回到寄存器，注意判断驱逐的是局部变量还是全局变量
            rv::rv_inst rv_save_Inst;
            int offset = getOffSetFromStackSpace(opd);
            rv_save_Inst.op = rv::rvOPCODE::SW;         // 操作
            rv_save_Inst.rs2 = reg;                     // 要驱逐的寄存器号
            rv_save_Inst.rs1 = rv::rvREG::X8;           // fp
            rv_save_Inst.imm = offset;
            fout << rv_save_Inst.draw();
        }
        i_opd2regTable.erase(opd);       // 驱逐到内存就要移除辣
        i_reg2opdTable.erase(reg);
        i_validReg &= ~(1 << regIndex);          // 掩蔽标志位
    }
    else if(caseOp == 1){    //浮点数
        rv::rvFREG reg = rv::rvFREG(regIndex);
        Operand opd = f_reg2opdTable[reg];
        if (isGlobalVar(opd)){
            fout<<"\t"<<"lui\t"<<toGenerateString(rv::rvREG::X7)<<","<<"\%hi("<<opd.name<<")"<<endl;
            fout<<"\t"<<"fsw\t"<<toGenerateString(reg)<<","<<"\%lo("<<opd.name<<")("<<toGenerateString(rv::rvREG::X7)<<")"<<endl;
            f_validReg &= ~(1 << regIndex);          // 掩蔽标志位
            f_reg2opdTable.erase(reg);
        }
        else{
            rv::rv_inst rv_save_Inst;
            int offset = getOffSetFromStackSpace(opd);
            rv_save_Inst.op = rv::rvOPCODE::FSW;         // 操作
            rv_save_Inst.frs2 = reg;                     // 要驱逐的寄存器号
            rv_save_Inst.rs1 = rv::rvREG::X8;            // fp
            rv_save_Inst.imm = offset;
            fout << rv_save_Inst.draw();
        }
        f_opd2regTable.erase(opd);       // 驱逐到内存就要移除辣
        f_reg2opdTable.erase(reg);
        f_validReg &= ~(1 << regIndex); // 掩蔽标志位
    }
    else{
        assert(0 && "Invalid optype!");
    }
}       // 将寄存器里面的值移回到内存，注意可能是全局变量

void backend::Generator::loadMemData(int regIndex, ir::Operand op){
    if (op.type == Type::Float  || op.type == Type::FloatPtr || op.type == Type::FloatLiteral){
        rv::rvFREG reg = rv::rvFREG(regIndex);
        assert(!(f_validReg >> regIndex) && "Not a empty register!");
        if (isGlobalVar(op)){   // 全局变量，仍然约定X7存地址
            fout<<"\t"<<"lui\t"<<toGenerateString(rv::rvREG::X7)<<","<<"\%hi("<<op.name<<")"<<endl;
            fout<<"\t"<<"flw\t"<<toGenerateString(reg)<<","<<"\%lo("<<op.name<<")("<<toGenerateString(rv::rvREG::X7)<<")"<<endl;
        }       // 局部变量
        else{
            // 读取进来
            rv::rv_inst rv_load_inst;
            rv_load_inst.op = rv::rvOPCODE::FLW;
            rv_load_inst.frd = reg;
            rv_load_inst.rs1 = rv::rvREG::X8;           // fp
            rv_load_inst.imm = getOffSetFromStackSpace(op);
            fout << rv_load_inst.draw();
        }
        f_validReg |= (1 << regIndex);          // 设置标志位
        f_opd2regTable[op] = reg;
        f_reg2opdTable[reg] = op;
    }
    else if (op.type == Type::IntPtr  || op.type == Type::Int || op.type == Type::IntLiteral){
        rv::rvREG reg = rv::rvREG(regIndex);
        assert(!(i_validReg >> regIndex) && "Not a empty register!");
        if (isGlobalVar(op)){   // 全局变量，约定X7放地址
            fout<<"\t"<<"lui\t"<<toGenerateString(rv::rvREG::X7)<<","<<"\%hi("<<op.name<<")"<<endl;
            fout<<"\t"<<"lw\t"<<toGenerateString(reg)<<","<<"\%lo("<<op.name<<")("<<toGenerateString(rv::rvREG::X7)<<")"<<endl;
        }
        else{
            // 读取进来
            rv::rv_inst rv_load_inst;
            rv_load_inst.op = rv::rvOPCODE::FLW;
            rv_load_inst.rd = reg;
            rv_load_inst.rs1 = rv::rvREG::X8;       // fp
            rv_load_inst.imm = getOffSetFromStackSpace(op);
            fout << rv_load_inst.draw();
        }
        i_validReg |= (1 << regIndex);          // 设置标志位
        i_opd2regTable[op] = reg;
        i_reg2opdTable[reg] = op;
    }
    else{
        assert(0 && "Unexpected Type!");
    }
}         // 将内存里面的值读进到寄存器

// 认为是给这个Operand分配一个合理的寄存器位置
// reg allocate api
// 所有的api要考虑以下问题：
// 对于新出现的变量，如何考虑？
//      - 在【临时寄存器】里面找一个没满的放进去，如果
//             满了，随机驱逐一个变量到内存，如果：
//                  能查到，那么直接写进去
//                  不能查到，那扩大栈空间
//             没满，那就选没满的那个
// 对于已经出现过的变量，如何调度？
//      - 查看是否在寄存器中，如果
//              在，则直接取出来
//              不在，则在表里搜，搜不到，报error
// 【写程序的时候一定要注意全局变量的情况】
// 记住tem0要做特判

// TODO: 检查变量是否是全局变量，如果不在栈中和寄存器中的话
// 同时如果驱逐的话，检查其是不是全局变量，如果是数组的话，那这将是个值得思考的问题
// 其实这里有个很有意思的算法，有点类似于写回的感觉：
//      冷缺失：即新定义的变量如def和fdef，根据前两个实验，这种变量肯定马上要用到，所以我们就有了
//      关于要换出的变量，其实是要看是全局变量还是局部变量，找对位置再sw
//      值得关注的是全局变量的同步问题
//      我们在驱逐寄存器的时候要检查该变量是不是全局变量
//      在函数进入退出的时候查看是不是全局变量
//      注意原子计算的问题，这些函数里面，如果是临时地用，那么用后就取消掉要用的占位符
//      如果是全局变量，哪个存地址？当然是选中的那个寄存器存地址辣！
//  【注意】：这个函数不执行赋值到新寄存器的操作
rv::rvREG  backend::Generator::getRd(ir::Operand op){
    assert(((op.type != Type::Float) && (op.type != Type::FloatLiteral)) && "Invalid float type detected.");
    static const std::vector<rv::rvREG> tempRegList{rv::rvREG::X5, rv::rvREG::X6};
    rv::rvREG ans = rv::rvREG::X32;
    if (isInReg(op)){       // 就在寄存器
        return i_opd2regTable[op];
    }
    else if (isGlobalVar(op) || isInStack(op)){
        for (auto reg : tempRegList){       // 这里可能还会有优化
            if (!((i_imAtomicComp >> (int)reg) & 1)){   // 不是当前计算要用到的寄存器
                if ((i_validReg >> int(reg)) & 1){      // 当前寄存器有值
                    expireRegData(int(reg), 0);
                }
                loadMemData(int(reg), op);
                i_imAtomicComp |= 1 << int(reg);
                ans = reg;
                break;
            }
        }
    }
    else{       // 全新变量，在栈中预先分配空间并写入寄存器
        for (auto reg : tempRegList){       // 这里可能还会有优化
            if (!((i_imAtomicComp >> (int)reg) & 1)){   // 不是当前计算要用到的寄存器
                if ((i_validReg >> int(reg)) & 1){      // 当前寄存器有值
                    expireRegData(int(reg), 0);
                }
                i_imAtomicComp |= 1 << int(reg);
                ans = reg;
                break;
            }
        }
        add_operand(op);        // 不用担心数组，他会自己分配
    }
    assert(ans != rv::rvREG::X32 && "Register Allocation Failed!");
    return ans;
}

rv::rvREG  backend::Generator::getRs1(ir::Operand op){
    assert(((op.type != Type::Float) && (op.type != Type::FloatLiteral)) && "Invalid float type detected.");
    static const std::vector<rv::rvREG> tempRegList{rv::rvREG::X28, rv::rvREG::X29};
    rv::rvREG ans = rv::rvREG::X32;
    if (isInReg(op)){       // 就在寄存器
        return i_opd2regTable[op];
    }
    else if (isGlobalVar(op) || isInStack(op)){
        for (auto reg : tempRegList){       // 这里可能还会有优化
            if (!((i_imAtomicComp >> (int)reg) & 1)){   // 不是当前计算要用到的寄存器
                if ((i_validReg >> int(reg)) & 1){      // 当前寄存器有值
                    expireRegData(int(reg), 0);
                }
                loadMemData(int(reg), op);
                i_imAtomicComp |= 1 << int(reg);
                ans = reg;
                break;
            }
        }
    }
    else{       // 全新变量，在栈中预先分配空间并写入寄存器
        for (auto reg : tempRegList){       // 这里可能还会有优化
            if (!((i_imAtomicComp >> (int)reg) & 1)){   // 不是当前计算要用到的寄存器
                if ((i_validReg >> int(reg)) & 1){      // 当前寄存器有值
                    expireRegData(int(reg), 0);
                }
                i_imAtomicComp |= 1 << int(reg);
                ans = reg;
                break;
            }
        }
        add_operand(op);        // 不用担心数组，他会自己分配
    }
    assert(ans != rv::rvREG::X32 && "Register Allocation Failed!");
    return ans;
}

rv::rvREG  backend::Generator::getRs2(ir::Operand op){
    assert(((op.type != Type::Float) && (op.type != Type::FloatLiteral)) && "Invalid float type detected.");
    static const std::vector<rv::rvREG> tempRegList{rv::rvREG::X30, rv::rvREG::X31};
    rv::rvREG ans = rv::rvREG::X32;
    if (isInReg(op)){       // 就在寄存器
        return i_opd2regTable[op];
    }
    else if (isGlobalVar(op) || isInStack(op)){
        for (auto reg : tempRegList){       // 这里可能还会有优化
            if (!((i_imAtomicComp >> (int)reg) & 1)){   // 不是当前计算要用到的寄存器
                if ((i_validReg >> int(reg)) & 1){      // 当前寄存器有值
                    expireRegData(int(reg), 0);
                }
                loadMemData(int(reg), op);
                i_imAtomicComp |= 1 << int(reg);
                ans = reg;
                break;
            }
        }
    }
    else{       // 全新变量，在栈中预先分配空间并写入寄存器
        for (auto reg : tempRegList){       // 这里可能还会有优化
            if (!((i_imAtomicComp >> (int)reg) & 1)){   // 不是当前计算要用到的寄存器
                if ((i_validReg >> int(reg)) & 1){      // 当前寄存器有值
                    expireRegData(int(reg), 0);
                }
                i_imAtomicComp |= 1 << int(reg);
                ans = reg;
                break;
            }
        }
        add_operand(op);        // 不用担心数组，他会自己分配
    }
    assert(ans != rv::rvREG::X32 && "Register Allocation Failed!");
    return ans;
}

rv::rvFREG backend::Generator::fgetRd(ir::Operand op){
    assert(((op.type != Type::Int) && (op.type != Type::IntLiteral) && (op.type != Type::IntPtr)  && (op.type != Type::FloatPtr)) && "Invalid float type detected.");
    static const std::vector<rv::rvFREG> tempRegList{rv::rvFREG::F0, rv::rvFREG::F1, rv::rvFREG::F2, rv::rvFREG::F3};
    rv::rvFREG ans = rv::rvFREG::F32;
    if (isInReg(op)){       // 就在寄存器
        return f_opd2regTable[op];
    }
    else if (isGlobalVar(op) || isInStack(op)){
        for (auto reg : tempRegList){       // 这里可能还会有优化
            if (!((f_imAtomicComp >> (int)reg) & 1)){   // 不是当前计算要用到的寄存器
                if ((f_validReg >> int(reg)) & 1){      // 当前寄存器有值
                    expireRegData(int(reg), 1);
                }
                loadMemData(int(reg), op);
                f_imAtomicComp |= 1 << int(reg);
                ans = reg;
                break;
            }
        }
    }
    else{       // 全新变量，在栈中预先分配空间并写入寄存器
        for (auto reg : tempRegList){       // 这里可能还会有优化
            if (!((f_imAtomicComp >> (int)reg) & 1)){   // 不是当前计算要用到的寄存器
                if ((f_validReg >> int(reg)) & 1){      // 当前寄存器有值
                    expireRegData(int(reg), 1);
                }
                f_imAtomicComp |= 1 << int(reg);
                ans = reg;
                break;
            }
        }
        add_operand(op);        // 不用担心数组，他会自己分配
    }
    assert(ans != rv::rvFREG::F32 && "Register Allocation Failed!");
    return ans;
}

rv::rvFREG backend::Generator::fgetRs1(ir::Operand op){
    assert(((op.type != Type::Int) && (op.type != Type::IntLiteral) && (op.type != Type::IntPtr)  && (op.type != Type::FloatPtr)) && "Invalid float type detected.");
    static const std::vector<rv::rvFREG> tempRegList{rv::rvFREG::F4, rv::rvFREG::F5, rv::rvFREG::F6, rv::rvFREG::F7};
    rv::rvFREG ans = rv::rvFREG::F32;
    if (isInReg(op)){       // 就在寄存器
        return f_opd2regTable[op];
    }
    else if (isGlobalVar(op) || isInStack(op)){
        for (auto reg : tempRegList){       // 这里可能还会有优化
            if (!((f_imAtomicComp >> (int)reg) & 1)){   // 不是当前计算要用到的寄存器
                if ((f_validReg >> int(reg)) & 1){      // 当前寄存器有值
                    expireRegData(int(reg), 1);
                }
                loadMemData(int(reg), op);
                f_imAtomicComp |= 1 << int(reg);
                ans = reg;
                break;
            }
        }
    }
    else{       // 全新变量，在栈中预先分配空间并写入寄存器
        for (auto reg : tempRegList){       // 这里可能还会有优化
            if (!((f_imAtomicComp >> (int)reg) & 1)){   // 不是当前计算要用到的寄存器
                if ((f_validReg >> int(reg)) & 1){      // 当前寄存器有值
                    expireRegData(int(reg), 1);
                }
                f_imAtomicComp |= 1 << int(reg);
                ans = reg;
                break;
            }
        }
        add_operand(op);        // 不用担心数组，他会自己分配
    }
    assert(ans != rv::rvFREG::F32 && "Register Allocation Failed!");
    return ans;
}

rv::rvFREG backend::Generator::fgetRs2(ir::Operand op){
    assert(((op.type != Type::Int) && (op.type != Type::IntLiteral) && (op.type != Type::IntPtr)  && (op.type != Type::FloatPtr)) && "Invalid float type detected.");
    static const std::vector<rv::rvFREG> tempRegList{rv::rvFREG::F28, rv::rvFREG::F29, rv::rvFREG::F30, rv::rvFREG::F31};
    rv::rvFREG ans = rv::rvFREG::F32;
    if (isInReg(op)){       // 就在寄存器
        return f_opd2regTable[op];
    }
    else if (isGlobalVar(op) || isInStack(op)){
        for (auto reg : tempRegList){       // 这里可能还会有优化
            if (!((f_imAtomicComp >> (int)reg) & 1)){   // 不是当前计算要用到的寄存器
                if ((f_validReg >> int(reg)) & 1){      // 当前寄存器有值
                    expireRegData(int(reg), 1);
                }
                loadMemData(int(reg), op);
                f_imAtomicComp |= 1 << int(reg);
                ans = reg;
                break;
            }
        }
    }
    else{       // 全新变量，在栈中预先分配空间并写入寄存器
        for (auto reg : tempRegList){       // 这里可能还会有优化
            if (!((f_imAtomicComp >> (int)reg) & 1)){   // 不是当前计算要用到的寄存器
                if ((f_validReg >> int(reg)) & 1){      // 当前寄存器有值
                    expireRegData(int(reg), 1);
                }
                f_imAtomicComp |= 1 << int(reg);
                ans = reg;
                break;
            }
        }
        add_operand(op);        // 不用担心数组，他会自己分配
    }
    assert(ans != rv::rvFREG::F32 && "Register Allocation Failed!");
    return ans;
}

// generate wrapper function
// 注意function那个数组的第一个一定是全局变量函数
void backend::Generator::gen() {
    // 产生全局变量
    backend::Generator::gen_globalVal();
    // 提示进入代码区
    fout << "\t.text" << endl;
    for (auto& func: program.functions){
        backend::Generator::gen_func(func);
    }
}

void backend::Generator::gen_func(ir::Function& func){
    // 每进入一个块，寄存器caller保存寄存器有效位清空
    // 进入一个函数时，要注意形参列表相关信息的维护（比如op到reg之间的映射！！！）
    // 需要计算sp的偏移量，并在退出时恢复回来，
    // 函数的最后，一定是有个jr指令跳出函数！！不是说碰到ir指令的return就寄了！
    // 初始化标注信息
    stackVarMap *svm = new stackVarMap();
    memvar_Stack.push_back(*svm);
    f_opd2regTable.clear();
    f_reg2opdTable.clear();
    i_opd2regTable.clear();
    i_reg2opdTable.clear();
    i_validReg = 0;
    f_validReg = 0;
    i_imAtomicComp = 0;
    f_imAtomicComp = 0;
    // 输出提示信息
    fout << "\t.globl\t" << func.name << endl;
    fout << "\t.type\t" << func.name << ",\t@function"<< endl;
    fout << func.name << ":"<<endl;     // 标签标注
    // 被调用者保存寄存器
    get_ir_flagInfo(func.InstVec);
    // 输出标题flag
    int callerRegSize = calleeRegisterSave() * 4;
    // 计算sp的移动
    int totSpace = callerRegSize + getStackSpaceSize(func.InstVec);
    rv::rv_inst rvInstSp;
    rvInstSp.op = rv::rvOPCODE::ADDI;
    rvInstSp.rd = rv::rvREG::X2;
    rvInstSp.rs1 = rv::rvREG::X2;
    rvInstSp.imm = totSpace;
    fout << rvInstSp.draw();
    for (auto inst: func.InstVec){
        gen_instr(*inst);
    }
    // 恢复sp
    i_imAtomicComp = 0;
    rvInstSp.imm = -totSpace;
    fout << rvInstSp.draw();
    calleeRegisterRestore();
    rv::rv_inst rvInstJR;
    TODO;   // 写返回
}

void backend::Generator::gen_instr(ir::Instruction& inst){
    // 库函数
    // 吐了，写完才发现写炸了：
    // 1、浮点数寄存器不能加载常量，要经过两个步骤
    // 2、所有的操作数要仔细辨别，这个操作数是不是全局变量，就算有的是Int，也要用IntPtr去访问到全局变量的地址！！吐了
    // 3、想到再补，现在头脑一片乱麻了，草了啊害
    // 4、仔细思考：全局变量作为源操作数和目的操作数怎么办，全局变量作为参数传进来怎么办（尤其是数组）
    // 5、完蛋
    // TODO DEBUG: 浮点计数器不能采用LI加载常量
    ir_stamp += 1;
    if (index_flag[ir_stamp]){
        fout << ".L" + std::to_string(index_flag[ir_stamp]) + ":"<< endl;
    }
    // 原子计算标志位清空
    i_imAtomicComp = 0;
    f_imAtomicComp = 0;
    ir::Operator op = inst.op;
    ir::Operand op1 = inst.op1;
    ir::Operand op2 = inst.op2;
    ir::Operand des = inst.des;
    switch (op)
    {
        // data assignment
        case ir::Operator::def:     // 认为马上就会用，就放寄存器吧...也要注意全局变量的情况
        case ir::Operator::mov:
        {
            rv::rv_inst inst;
            inst.rd = getRd(des);
            if (op1.type == Type::IntLiteral){
                inst.imm = (uint32_t)std::stoi(op1.name);
                inst.op = rv::rvOPCODE::LI;
            }
            else{
                inst.rs1 = getRs1(op1);
                inst.op = rv::rvOPCODE::MOV;
            }
            fout << inst.draw();
        }
            break;
        case ir::Operator::fdef:
        case ir::Operator::fmov:
        {
            rv::rv_inst inst;
            inst.frd = fgetRd(des);
            if (op1.type == Type::FloatLiteral){
                // 小心这里丢精度，草，浮点数要经过：LI, fmv.w.x两个阶段即可
                float f = std::stof(op1.name);
                rv::rv_inst rvInst_LI;
                rvInst_LI.op = rv::rvOPCODE::LI;
                inst.rs1 = rvInst_LI.rd = getRd(op1);
                rvInst_LI.imm = (*(uint32_t*)(&f));
                fout << rvInst_LI.draw();
                inst.op = rv::rvOPCODE::FMV_W_X;
            }
            else{
                inst.frs1 = fgetRs1(op1);
                inst.op = rv::rvOPCODE::FMOV;
            }
            fout << inst.draw();
        }
            break;
        // arithmetic computing
        case ir::Operator::add:
        {
            assert(!(op1.type == Type::IntLiteral && op2.type == Type::IntLiteral) && "两个字面量你算你妈呢。");
            rv::rv_inst rvInst;
            // 三种情况：有一个是字面量；全是临时变量
            rvInst.rd = getRd(des);
            if (op1.type == Type::IntLiteral){
                rvInst.rs1 = getRs1(op2);
                rvInst.imm = std::stoi(op1.name);
                rvInst.op = rv::rvOPCODE::ADDI;
            }
            else if(op2.type == Type::IntLiteral){
                rvInst.rs1 = getRs1(op1);
                rvInst.imm = std::stoi(op2.name);
                rvInst.op = rv::rvOPCODE::ADDI;
            }
            else{
                rvInst.rs1 = getRs1(op1);
                rvInst.rs2 = getRs2(op2);
                rvInst.op = rv::rvOPCODE::ADD;
            }
            fout << rvInst.draw();
        }
            break;
        case ir::Operator::addi:
        case ir::Operator::subi:
        {
            rv::rv_inst rvInst;
            rvInst.op = rv::rvOPCODE::ADDI;
            rvInst.rd = getRd(des);
            rvInst.rs1 = getRs1(op1);
            rvInst.imm = op == Operator::addi ? (uint32_t)std::stoi(op2.name) : -(uint32_t)std::stoi(op2.name);
            fout << rvInst.draw();
        }
            break;
        case ir::Operator::fadd:
        case ir::Operator::fsub:
        case ir::Operator::fmul:
        case ir::Operator::fdiv:
        case ir::Operator::flss:    // <        FLT
        case ir::Operator::fleq:    // <=       FLE
        case ir::Operator::fgtr:    // >        not exist
        case ir::Operator::fgeq:    // >=       not exist
        case ir::Operator::feq:     // ==
        case ir::Operator::fneq:    // !=
        {
            rv::rv_inst rvInst;
            // 四种情况
            rvInst.frd = fgetRd(des);
            rvInst.op = op == Operator::fadd ? rv::rvOPCODE::FADD_S :
                        op == Operator::fsub ? rv::rvOPCODE::FSUB_S :
                        op == Operator::fmul ? rv::rvOPCODE::FMUL_S :
                        op == Operator::fdiv ? rv::rvOPCODE::FDIV_S :
                        op == Operator::flss ? rv::rvOPCODE::FLT_S :
                        op == Operator::fleq ? rv::rvOPCODE::FLE_S :
                        op == Operator::feq  ? rv::rvOPCODE::FEQ_S :
                        op == Operator::fneq ? rv::rvOPCODE::FNEQ_S:
                        op == Operator::fgtr ? rv::rvOPCODE::FLT_S : rv::rvOPCODE::FLE_S;
            if (op == Operator::fgtr || op == Operator::fgeq){
                std::swap(op1, op2);
            }
            if (op1.type == Type::FloatLiteral && op2.type == Type::FloatLiteral){
                float f1 = std::stof(op1.name);
                float f2 = std::stof(op2.name);
                int32_t u1 = *(int32_t *)(&f1);
                int32_t u2 = *(int32_t *)(&f2);
                rv::rv_inst rvInst_li;
                rv::rv_inst rvInst_ifmov;
                rvInst_li.op = rv::rvOPCODE::LI;
                rvInst_ifmov.op = rv::rvOPCODE::FMV_W_X;
                // 处理op1
                rvInst_li.imm = u1;
                rvInst_ifmov.rs1 = rvInst_li.rd = getRd(op1);
                fout << rvInst_li.draw();
                rvInst.frs1 = rvInst_ifmov.frd = fgetRd(op1);
                fout << rvInst_li.draw();
                // 处理op2
                rvInst_li.imm = u2;
                rvInst_ifmov.rs1 = rvInst_li.rd = getRd(op2);
                fout << rvInst_li.draw();
                rvInst.frs1 = rvInst_ifmov.frd = fgetRd(op2);
                fout << rvInst_li.draw();
            }
            else if (op1.type == Type::FloatLiteral){
                float f1 = std::stof(op1.name);
                int32_t u1 = *(int32_t *)(&f1);
                rv::rv_inst rvInst_li;
                rv::rv_inst rvInst_ifmov;
                rvInst_li.op = rv::rvOPCODE::LI;
                rvInst_ifmov.op = rv::rvOPCODE::FMV_W_X;
                // 处理op1
                rvInst_li.imm = u1;
                rvInst_ifmov.rs1 = rvInst_li.rd = getRd(op1);
                fout << rvInst_li.draw();
                rvInst.frs1 = rvInst_ifmov.frd = fgetRd(op1);
                fout << rvInst_li.draw();
                // 得到op2
                rvInst.frs2 = fgetRs2(op2);
            }
            else if (op2.type == Type::FloatLiteral){
                float f2 = std::stof(op2.name);
                int32_t u2 = *(int32_t *)(&f2);
                rv::rv_inst rvInst_li;
                rv::rv_inst rvInst_ifmov;
                rvInst_li.op = rv::rvOPCODE::LI;
                rvInst_ifmov.op = rv::rvOPCODE::FMV_W_X;
                // 处理op2
                rvInst_li.imm = u2;
                rvInst_ifmov.rs1 = rvInst_li.rd = getRd(op2);
                fout << rvInst_li.draw();
                rvInst.frs1 = rvInst_ifmov.frd = fgetRd(op2);
                fout << rvInst_li.draw();
                // 得到op1
                rvInst.frs1 = fgetRs1(op1);
            }
            else{
                rvInst.frs1 = fgetRs1(op1);
                rvInst.frs2 = fgetRs2(op2);
            }
            fout << rvInst.draw();
        }
            break;
        case ir::Operator::sub:
        {
            assert(!(op1.type == Type::IntLiteral && op2.type == Type::IntLiteral) && "两个字面量你算你妈呢。");
            rv::rv_inst rvInst;
            // 三种情况：有一个是字面量；全是临时变量
            rvInst.rd = getRd(des);
            if (op1.type == Type::IntLiteral){      // op1是intLiteral，有点麻烦
                rv::rv_inst rvInst_li;
                rvInst_li.op = rv::rvOPCODE::LI;
                rvInst.rs1 = rvInst_li.rs1 = getRs1(op1);
                rvInst.rs2 = getRs2(op2);
                rvInst.op = rv::rvOPCODE::SUB;
            }
            else if(op2.type == Type::IntLiteral){
                rv::rv_inst rvInst;
                rvInst.op = rv::rvOPCODE::ADDI;
                rvInst.rd = getRd(des);
                rvInst.rs1 = getRs1(op1);
                rvInst.imm = -(uint32_t)std::stoi(op2.name);
                fout << rvInst.draw();
            }
            else{
                rvInst.rs1 = getRs1(op1);
                rvInst.rs2 = getRs2(op2);
                rvInst.op = rv::rvOPCODE::SUB;
            }
            fout << rvInst.draw();
        }
            break;
        case ir::Operator::mul:
        case ir::Operator::div:
        case ir::Operator::mod:
        {
            rv::rv_inst rvInst;
            // 四种情况：Int和Literal两两组合
            // 由于没有faddi，所以要迁移一下，多个li的伪指令
            // 我们约定第一个算寄存器的，第二个是立即数加载出来的
            rvInst.rd = getRd(des);
            rvInst.op = op == Operator::mul ? rv::rvOPCODE::MUL :\
                        op == Operator::div ? rv::rvOPCODE::DIV : rv::rvOPCODE::REM;
            if (op1.type == Type::IntLiteral && op2.type == Type::IntLiteral){
                int32_t u1 = std::stoi(op1.name);
                int32_t u2 = std::stoi(op2.name);
                rv::rv_inst rvInst_li;
                rvInst_li.op = rv::rvOPCODE::LI;
                rvInst.rs1 = rvInst_li.rd = getRs1(op1);
                rvInst_li.imm = u1;
                fout << rvInst_li.draw();
                rvInst.rs2 = rvInst_li.rd = getRs2(op2);
                rvInst_li.imm = u2;
                fout << rvInst_li.draw();
            }
            else if (op1.type == Type::IntLiteral){
                int32_t u1 = std::stoi(op1.name);
                rv::rv_inst rvInst_li;
                rvInst_li.op = rv::rvOPCODE::LI;
                rvInst.rs1 = rvInst_li.rd = getRs1(op1);
                rvInst_li.imm = u1;
                fout << rvInst_li.draw();
                rvInst.rs2 = getRs2(op2);
            }
            else if (op2.type == Type::IntLiteral){
                int32_t u2 = std::stoi(op2.name);
                rv::rv_inst rvInst_li;
                rvInst_li.op = rv::rvOPCODE::LI;
                rvInst.rs2 = rvInst_li.rd = getRs2(op2);
                rvInst_li.imm = u2;
                fout << rvInst_li.draw();
                rvInst.rs1 = getRs1(op1);
            }
            else{
                rvInst.rs1 = getRs1(op1);
                rvInst.rs2 = getRs2(op2);
            }
            fout << rvInst.draw();
        }
            break;
        // comparison instruction
        case ir::Operator::lss:     // <        BLT
        case ir::Operator::leq:     // <=       not exist
        case ir::Operator::gtr:     // >        not exist
        case ir::Operator::geq:     // >=       BGE
        case ir::Operator::eq:      // ==       BEQ
        case ir::Operator::neq:     // !=       BNE
        {
            rv::rv_inst rvInst;
            // 四种情况：Int和Literal两两组合
            // 由于没有faddi，所以要迁移一下，多个li的伪指令
            // 我们约定第一个算寄存器的，第二个是立即数加载出来的
            rvInst.rd = getRd(des);
            rvInst.op = op == Operator::lss ? rv::rvOPCODE::BLT :\
                        op == Operator::leq ? rv::rvOPCODE::BGE :\
                        op == Operator::gtr ? rv::rvOPCODE::BLT :\
                        op == Operator::geq ? rv::rvOPCODE::BGE :\
                        op == Operator::eq ? rv::rvOPCODE::BEQ : rv::rvOPCODE::BNE;
            // 两种情况，特殊交换一下
            if (op == Operator::leq || op == Operator::gtr){
                std::swap(op1, op2);
            }
            if (op1.type == Type::IntLiteral && op2.type == Type::IntLiteral){
                int32_t u1 = std::stoi(op1.name);
                int32_t u2 = std::stoi(op2.name);
                rv::rv_inst rvInst_li;
                rvInst_li.op = rv::rvOPCODE::LI;
                rvInst.rs1 = rvInst_li.rd = getRs1(op1);
                rvInst_li.imm = u1;
                fout << rvInst_li.draw();
                rvInst.rs2 = rvInst_li.rd = getRs2(op2);
                rvInst_li.imm = u2;
                fout << rvInst_li.draw();
            }
            else if (op1.type == Type::IntLiteral){
                int32_t u1 = std::stoi(op1.name);
                rv::rv_inst rvInst_li;
                rvInst_li.op = rv::rvOPCODE::LI;
                rvInst.rs1 = rvInst_li.rd = getRs1(op1);
                rvInst_li.imm = u1;
                fout << rvInst_li.draw();
                rvInst.rs2 = getRs2(op2);
            }
            else if (op2.type == Type::IntLiteral){
                int32_t u2 = std::stoi(op2.name);
                rv::rv_inst rvInst_li;
                rvInst_li.op = rv::rvOPCODE::LI;
                rvInst.rs2 = rvInst_li.rd = getRs2(op2);
                rvInst_li.imm = u2;
                fout << rvInst_li.draw();
                rvInst.rs1 = getRs1(op1);
            }
            else{
                rvInst.rs1 = getRs1(op1);
                rvInst.rs2 = getRs2(op2);
            }
            fout << rvInst.draw();
        }
            break;
        // logic instruction
        case ir::Operator::_not:        // 根据文档，必然是int类型
        {
            rv::rv_inst rvInst;
            rvInst.op = rv::rvOPCODE::BEQ;      // 和零寄存器判断
            rvInst.rd = getRd(des);
            rvInst.rs1 = getRs1(op1);
            rvInst.rs2 = rv::rvREG::X0;
            fout << rvInst.draw();
        }
            break;
        case ir::Operator::_and:        // 根据文档，必然是int类型
        case ir::Operator::_or:
        {
            rv::rv_inst rvInst;
            // 四种情况：Int和Literal两两组合
            // 由于没有faddi，所以要迁移一下，多个li的伪指令
            // 我们约定第一个算寄存器的，第二个是立即数加载出来的
            rvInst.rd = getRd(des);
            rvInst.op = op == Operator::_or ? rv::rvOPCODE::OR : rv::rvOPCODE::AND;
            if (op1.type == Type::IntLiteral || op2.type == Type::IntLiteral){
                rvInst.op = op == Operator::_or ? rv::rvOPCODE::ORI : rv::rvOPCODE::ANDI;
            }
            if (op1.type == Type::IntLiteral && op2.type == Type::IntLiteral){
                int32_t u1 = std::stoi(op1.name);       // op1的进寄存器，op2是常量
                int32_t u2 = std::stoi(op2.name);
                rv::rv_inst rvInst_li;
                rvInst_li.op = rv::rvOPCODE::LI;
                rvInst.rs1 = rvInst_li.rd = getRs1(op1);
                rvInst_li.imm = u1;
                fout << rvInst_li.draw();
                rvInst.imm = u2;
            }
            else if (op1.type == Type::IntLiteral){
                int32_t u1 = std::stoi(op1.name);
                rvInst.rs1 = getRs1(op2);
                rvInst.imm = u1;
            }
            else if (op2.type == Type::IntLiteral){
                int32_t u2 = std::stoi(op2.name);
                rvInst.rs1 = getRs1(op1);
                rvInst.imm = u2;
            }
            else{
                rvInst.rs1 = getRs1(op1);
                rvInst.rs2 = getRs2(op2);
            }
            fout << rvInst.draw();
        }
            break;
        // lw operation and alloc, getptr
        case ir::Operator::load:    // des 是目标读出变量，op1 是数组名，op2是偏移量
        // 这里就要小心考虑了
        // int a[3] = {1,2,3};
        // int b = a[2];
        // int c = a[1] + 9;
        // a[2]这里的翻译是：
        // getptr $tem, a, 2
        // 注意这里$tem是地址不是load出来的值
        // 注意FloatPtr的情况
        {
            // rs1 is a base, rd is target and imm is offset
            rv::rv_inst rvInst;
            if (op1.type == Type::FloatPtr){
                if (isGlobalVar(op1)){      // 全局变量
                    fout<<"\t"<<"lui\t"<<toGenerateString(rvInst.rd)<<","<<"\%hi("<<op1.name<<")"<<endl;
                    fout<<"\t"<<"addi\t"<<toGenerateString(rvInst.rd)<<","<<toGenerateString(rvInst.rd)<<","<<"\%lo("<<op1.name<<")"<<endl;     // arrName base addr
                    // 此时rd存的是数组基址
                    rvInst.rs1 = rvInst.rd;
                    rvInst.imm = 4 * std::stoi(op2.name);
                    rvInst.op = rv::rvOPCODE::FLW;
                    fout << rvInst.draw();
                }
                else if (isInStack(op1)){
                    rvInst.op = rv::rvOPCODE::FLW;
                    rvInst.frd = fgetRd(des);
                    rvInst.rs1 = rv::rvREG::X8;
                    int arr_base_offset = getOffSetFromStackSpace(op1);
                    int arr_idx_offset = std::stoi(op2.name) * 4;
                    rvInst.imm = arr_base_offset + arr_idx_offset ;
                    fout << rvInst.draw();
                }
                else{
                    assert(0 && "No such a operand!");
                }
            }
            else if (op1.type == Type::IntPtr){
                if (isGlobalVar(op1)){
                    fout<<"\t"<<"lui\t"<<toGenerateString(rvInst.rd)<<","<<"\%hi("<<op1.name<<")"<<endl;
                    fout<<"\t"<<"addi\t"<<toGenerateString(rvInst.rd)<<","<<toGenerateString(rvInst.rd)<<","<<"\%lo("<<op1.name<<")"<<endl;     // arrName base addr
                    rvInst.rs1 = rvInst.rd;
                    rvInst.imm = 4 * std::stoi(op2.name);
                    rvInst.op = rv::rvOPCODE::LW;
                    fout << rvInst.draw();
                }
                else if (isInStack(op1)){
                    rvInst.op = rv::rvOPCODE::LW;
                    rvInst.rd = getRd(des);
                    rvInst.rs1 = rv::rvREG::X8;
                    int arr_base_offset = getOffSetFromStackSpace(op1);
                    int arr_idx_offset = std::stoi(op2.name) * 4;
                    rvInst.imm = arr_base_offset + arr_idx_offset ;
                    fout << rvInst.draw();
                }
                else{
                    assert(0 && "No such a operand!");
                }
            }
            else{
                assert(0 && "Invalid type!");
            }
        }
            break;
        case ir::Operator::store:   // des 是目标存入变量，op1 是数组名，op2是偏移量
        // 注意考虑全局变量的情况
        {
            rv::rv_inst rvInst;
            if (op1.type == Type::FloatPtr){
                if (isGlobalVar(op1)){      // 全局变量
                    rvInst.rs1 = getRs1(op1);
                    fout<<"\t"<<"lui\t"<<toGenerateString(rvInst.rs1)<<","<<"\%hi("<<op1.name<<")"<<endl;
                    fout<<"\t"<<"addi\t"<<toGenerateString(rvInst.rs1)<<","<<toGenerateString(rvInst.rd)<<","<<"\%lo("<<op1.name<<")"<<endl;     // arrName base addr
                    // 此时rs1存的是数组基址
                    rvInst.imm = 4 * std::stoi(op2.name);
                    rvInst.op = rv::rvOPCODE::FSW;
                    rvInst.frs2 = fgetRs2(des);
                    fout << rvInst.draw();
                }
                else if (isInStack(op1)){
                    rvInst.op = rv::rvOPCODE::FSW;
                    rvInst.frs2 = fgetRs2(des);      // 要存的值，注意一下
                    rvInst.rs1 = rv::rvREG::X8;
                    int arr_base_offset = getOffSetFromStackSpace(op1);
                    int arr_idx_offset = std::stoi(op2.name) * 4;
                    rvInst.imm = arr_base_offset + arr_idx_offset ;
                    fout << rvInst.draw();
                }
                else{
                    assert(0 && "No such a operand!");
                }
            }
            else if (op1.type == Type::IntPtr){
                if (isGlobalVar(op1)){
                    rvInst.rs1 = getRs1(op1);
                    fout<<"\t"<<"lui\t"<<toGenerateString(rvInst.rs1)<<","<<"\%hi("<<op1.name<<")"<<endl;
                    fout<<"\t"<<"addi\t"<<toGenerateString(rvInst.rs1)<<","<<toGenerateString(rvInst.rd)<<","<<"\%lo("<<op1.name<<")"<<endl;     // arrName base addr
                    // 此时rs1存的是数组基址
                    rvInst.imm = 4 * std::stoi(op2.name);
                    rvInst.op = rv::rvOPCODE::SW;
                    rvInst.rs2 = getRs2(des);
                    fout << rvInst.draw();
                }
                else if (isInStack(op1)){
                    rvInst.op = rv::rvOPCODE::SW;
                    rvInst.rs2 = getRs2(des);
                    rvInst.rs1 = rv::rvREG::X8;
                    int arr_base_offset = getOffSetFromStackSpace(op1);
                    int arr_idx_offset = std::stoi(op2.name) * 4;
                    rvInst.imm = arr_base_offset + arr_idx_offset;
                    fout << rvInst.draw();
                }
                else{
                    assert(0 && "No such a operand!");
                }
            }
            else{
                assert(0 && "Invalid type!");
            }
        }
            break;
        case ir::Operator::alloc:   // alloc des(a), op1(2)
        {
            int sz = std::stoi(op1.name);
            add_operand(des, sz * 4);
        }
            break;
        case ir::Operator::getptr:
        // getptr des(np), arrName(op1), off(op2)
        // 任务其实是获得地址
        // 要注意判断全局变量的情况
        // 想一下哈，这里有很重要的问题：
        // 对于全局变量来说，尼玛吗的又是大小端，但，riscv是小端，你他妈存在栈中的地址偏移错了就完蛋！
        // TODO: 全局变量：下标越大，地址越大，但是我们的栈空间，s0是地址大的位置，那么偏移量应该是负数
        // 那么我们的xx也要符合这个规定
        {
            rv::rv_inst rvInst;
            rvInst.rd = getRd(des);
            if (isGlobalVar(op1)){       // 全局变量
                fout<<"\t"<<"lui\t"<<toGenerateString(rvInst.rd)<<","<<"\%hi("<<op1.name<<")"<<endl;
                fout<<"\t"<<"addi\t"<<toGenerateString(rvInst.rd)<<","<<toGenerateString(rvInst.rd)<<","<<"\%lo("<<op1.name<<")"<<endl;     // arrName base addr
                fout<<"\t"<<"addi\t"<<toGenerateString(rvInst.rd)<<","<<toGenerateString(rvInst.rd)<<","<<std::stoi(op2.name) * 4<<endl;    // offset
            }
            else if (isInStack(op1)){   // 局部变量
                int arr_idx_offset = std::stoi(op2.name) * 4;
                int arr_base_offset = getOffSetFromStackSpace(op1);
                int totOffSet_fp = arr_idx_offset + arr_base_offset;
                rvInst.op = rv::rvOPCODE::ADDI;
                rvInst.rs1 = rv::rvREG::X8;
                rvInst.imm = totOffSet_fp;
                fout << rvInst.draw();
            }
            else{
                assert(0 && "No such a operand!");
            }
        }
            break;
        // function control
        case ir::Operator::call:    // 难度较大，想一下sp指针如何移动？在哪移动？调用前移动好吗？显然不好。但参数如何保存得知道
        {
            // call的返回值还没处理呢！
            ir::Instruction *instPtr = &inst;
            ir::CallInst* callInstPtr= dynamic_cast<ir::CallInst*>(instPtr);    // callInst 要向下类型转换
            assert(callInstPtr && "Not a callInst.");
            callerRegisterSave();
            // 把相应参数读入相应寄存器
            int intRegCount = 0;        // 整数寄存器的数量
            int floatRegCount = 0;      // 浮点数寄存器的数量
            static rv::rvREG i_reg_param[8] = {rv::rvREG::X10, rv::rvREG::X11, rv::rvREG::X12, \
                rv::rvREG::X13, rv::rvREG::X14, rv::rvREG::X15, rv::rvREG::X16, rv::rvREG::X17};
            static rv::rvFREG f_reg_param[8] = {rv::rvFREG::F10, rv::rvFREG::F11, rv::rvFREG::F12, \
                rv::rvFREG::F13, rv::rvFREG::F14, rv::rvFREG::F15, rv::rvFREG::F16, rv::rvFREG::F17};
            std::vector<ir::Operand> stackParamList;        // 存放装不下的全局变量到stack空间里，
            // 注意fp会到最后的sp，所以一定要相对于sp来存，我们假定sp在gen_func里面管理，
            // 且在该函数就可以算出sp
            for (size_t i = 0; i < callInstPtr->argumentList.size();i++){
                // 考虑全局变量和临时变量，以及六种参数的可能情况的处理？
                // 哈，浮点数寄存器也要传参的hhh
                // 问题来了：全局变量作为形参，在被调用者函数中，应该怎么识别
                // 识别你妈啊，傻逼，这他妈不就变成局部变量了么？
                ir::Operand paramOpd = callInstPtr->argumentList[i];
                if (paramOpd.type == Type::IntLiteral){     // 先考虑字面量的情况，他们必然不受全局变量的打扰
                    if (intRegCount >= 8){
                        stackParamList.push_back(paramOpd);
                        continue;
                    }
                    rv::rv_inst rvInst;
                    rvInst.op = rv::rvOPCODE::LI;
                    rvInst.rd = i_reg_param[intRegCount++];
                    rvInst.imm = std::stoi(paramOpd.name);
                    fout << rvInst.draw();
                }
                else if (paramOpd.type == Type::FloatLiteral){
                    if (floatRegCount >= 8){
                        stackParamList.push_back(paramOpd);
                        continue;
                    }
                    rv::rv_inst rvInst;
                    rv::rv_inst rvInst_f;
                    rvInst.op = rv::rvOPCODE::LI;
                    rvInst_f.op = rv::rvOPCODE::FMV_W_X;
                    rvInst_f.rs1 = rvInst.rd = rv::rvREG::X7;      // 一切都交给X7承担！
                    float f = std::stof(paramOpd.name);
                    rvInst.imm = *(int32_t *)(&f);
                    rvInst_f.frd = f_reg_param[floatRegCount++];
                    fout << rvInst.draw();
                    fout << rvInst_f.draw();
                }
                // 啊，其实全局变量没必要考虑，主要是getRd啥的都处理了
                else if (paramOpd.type == Type::Float){
                    if (floatRegCount >= 8){
                        stackParamList.push_back(paramOpd);
                        continue;
                    }
                    rv::rv_inst rvInst;
                    rvInst.op = rv::rvOPCODE::FMOV;
                    rvInst.frd = f_reg_param[floatRegCount++];
                    rvInst.frs1 = fgetRs1(paramOpd);
                    fout << rvInst.draw();
                }
                // 小坑，指针一定是整数类型
                else if (paramOpd.type == Type::Int || paramOpd.type == Type::IntPtr  || paramOpd.type == Type::FloatPtr){
                    if (intRegCount >= 8){
                        stackParamList.push_back(paramOpd);
                        continue;
                    }
                    rv::rv_inst rvInst;
                    rvInst.op = rv::rvOPCODE::MOV;
                    rvInst.rd = i_reg_param[floatRegCount++];
                    rvInst.rs1 = getRs1(paramOpd);
                    fout << rvInst.draw();
                }
                else{
                    assert(0 && "unexpected type!");
                }
            }
            // 处理参数个数溢出的情况
            for (auto& opd: stackParamList){
                if (stackParamList.size()){
                    // FIXME : 盲猜样例没有这种情况，有了assert了再看吧，不想写
                    assert(0 && "Need Process FuncParams which more than 8!");
                }
            }
            // 此时a0存放着返回值，需要load回寄存器
            rv::rv_inst rvInst;
            if (des.type == Type::Int){
                rvInst.op = rv::rvOPCODE::SW;
                rvInst.rs1 = rv::rvREG::X8;
                rvInst.rs2 = rv::rvREG::X10;
                rvInst.imm = add_operand(des);
                fout << rvInst.draw();
            }
            else if (des.type == Type::Float){
                rvInst.op = rv::rvOPCODE::FSW;
                rvInst.op = rv::rvOPCODE::SW;
                rvInst.rs1 = rv::rvREG::X8;
                rvInst.frs2 = rv::rvFREG::F10;
                rvInst.imm = add_operand(des);
                fout << rvInst.draw();
            }
            else{
                assert(des.type == Type::null && "Unexpeced Return Type!");
            }
            TODO;       // 处理函数跳转
            calleeRegisterRestore();        // 恢复寄存器
        }
            break;
        case ir::Operator::_return:
        {
            // 感觉要恢复现场了
            // 恢复现场的事就交给gen_func函数好了 
            // a0和a1是caller寄存器不是callee，返回时只读回来callee，所以不用怕
            rv::rv_inst rvInst;
            // 注意浮点数的返回值的存储地址
            if (op1.type == Type::Int){
                rv::rv_inst rvInst;
                rvInst.op = rv::rvOPCODE::MOV;
                rvInst.rd = rv::rvREG::X10;
                rvInst.rs1 = getRs1(op1);
                fout << rvInst.draw();
            }
            else if (op1.type == Type::Float){
                rv::rv_inst rvInst;
                rvInst.op = rv::rvOPCODE::FMOV;
                rvInst.frd = rv::rvFREG::F10;   // a0 返回地址
                rvInst.frs1 = fgetRs1(op1);
                fout << rvInst.draw();
            }
            else if (op1.type == Type::IntLiteral){
                rv::rv_inst rvInst;
                rvInst.op = rv::rvOPCODE::LI;
                rvInst.rd = rv::rvREG::X10;     // a0 返回地址
                rvInst.imm = std::stoi(op1.name);
                fout << rvInst.draw();
            }
            else if (op1.type == Type::FloatLiteral){
                rv::rv_inst rvInst;
                rv::rv_inst rvInst_f;
                rvInst.op = rv::rvOPCODE::LI;
                rvInst_f.op = rv::rvOPCODE::FMV_W_X;
                rvInst_f.rs1 = rvInst.rd = rv::rvREG::X7;      // 一切都交给X7承担！
                float f = std::stof(op1.name);
                rvInst.imm = *(int32_t *)(&f);
                rvInst_f.frd = rv::rvFREG::F10;     // 浮点数的返回寄存器
                fout << rvInst.draw();
                fout << rvInst_f.draw();
            }
            else{
                // 注意Ptr的情况没处理，我盲猜不需要处理，应该在IR阶段处理了，遇到了再说吧
                assert(op1.type == Type::null && "Unexpected Type!");
            }
        }
            break;
        case ir::Operator::_goto:
        // op1 cond des offset
        // 值得注意的是跳转多远？
        // 编译器给出了答案：定一个小标签，那么就转化成一个在线算法(fnmdp，只能离线处理，考虑while循环，你猜猜往回跳的flag怎么确定？)：
        // 维护一个set，每打印一个inst，set - 1，当set变为0时，打印一个flag。
        // 但是这个算法的时间复杂度明显很高，怎么办？
        // 不一定，对于一个函数，可以维护一个时间戳，这个时间戳每扫一个ir自增1，
        // 如果扫到一个goto，那么记录一个时间戳+goto的偏移量
        // 注意跳转到同一个地方的问题，这个问题的最好办法就是设定一个哈希表，每次进行查询。
        // 哈希表以index作为下标，存储的元素是flag编号。这个编号单调递增，全局唯一
        // 完美，只不过要注意每个函数都需要把哈希表清空，然后注意哈希表的回绕。
        {
            rv::rv_inst rvInst;
            assert(index_flag[ir_stamp] && "Invalid Flag!");
            rvInst.label = ".L" + std::to_string(index_flag[ir_stamp]);
            if (op1.type != Type::null){        // 有条件，注意都是可能跳标签的
                rv::rv_inst rvInst;
                if (op1.type == Type::IntLiteral){      // 整数字面量
                    if (std::stoi(op1.name)){   // 满足条件
                        rvInst.op = rv::rvOPCODE::J;
                        fout << rvInst.draw();
                    }
                }
                else if(op1.type == Type::FloatLiteral){    // 浮点数字面量
                    if (std::stof(op1.name)){   // 满足条件
                        rvInst.op = rv::rvOPCODE::J;
                        fout << rvInst.draw();
                    }
                }
                else if (op1.type == Type::Int){
                    rvInst.op = rv::rvOPCODE::BNE;
                    rvInst.rs1 = getRs1(op1);
                    rvInst.rs2 = rv::rvREG::X0;
                }
                else if (op1.type == Type::Float){  // 一眼不可能，先否了再说
                    assert(0 && "Goto: invalid float register!");
                }
                else{
                    assert(0 && "Invalid Type!");
                }
            }
            else{       // 无条件跳转
                rvInst.op = rv::rvOPCODE::J;
                fout << rvInst.draw();
            }
        }
            break;
        // convertion
        case ir::Operator::cvt_f2i:
        {
            rv::rv_inst rvInst;
            rvInst.op = rv::rvOPCODE::FCVT_W_S;
            rvInst.rd = getRd(des);
            rvInst.frs1 = fgetRs1(op1);
            fout << rvInst.draw();
        }
            break;
        case ir::Operator::cvt_i2f:
        {
            rv::rv_inst rvInst;
            rvInst.op = rv::rvOPCODE::FCVT_S_W;
            rvInst.frd = fgetRd(des);
            rvInst.rs1 = getRs1(op1);
            fout << rvInst.draw();
        }
            break;
        // unuse
        case ir::Operator::__unuse__:
        {
            rv::rv_inst rvInst;
            rvInst.op = rv::rvOPCODE::NOP;
            fout << rvInst.draw();
        }
            break;
        default:
            break;
    }
}

void backend::Generator::gen_globalVal(){
    // 注意有两种情况：看有没有初始化，如果有初始化行为，那么必然有store！
    // 主要是数组，数组的initializer如果为{}，有store（我去改一下，改个寄吧，算了，汇编讨论）
    // case 1: const 必有初始化
    // case 2: 没有const可能不初始化，可能有，但一旦有，那所有元素必有store或赋值，
    //         数组一定是mov, fdef, fmov或fdef给一个临时变量，然后用临时变量赋值给数组元素
    // case 3: 对于常量必定优化没了，但是变量的话，还是会有
    // case 4: 数组的initizer必然是编译期可计算出的数值
    // case 5: 全局变量的计算会复杂很多，小心方能使得万年船
    // case 6: int k = 3; int b = 9; int y = k * 9 + b; 对 y 如何处理呢？在riscv编译器的产生结果
    //         不太一样，如果c没有在后面用到，可以不用算，否则会算
    // 因此，有个比较大的问题是全局变量的计算问题，应该如何解决？
    // 全局变量的声明还是蛮难的，需要对globalFunc的指令建立控制流图，把值全部算出来。
    // 那么有哪些操作呢？load, store, (f)mov, (f)def, add*3, sub*3, mul*2, div*2
    // 注意可能会调用函数
            // 可能的优化，先不写了
            // std::map <std::string, backend::GlobalValElement> globalValTable;
            // // 初始化全局变量表
            // for (auto ele: program.globalVal){
            //     Operand opd = ele.val;
            //     std::string sVarName = opd.name;
            //     Type tp = opd.type;
            //     int maxLen = ele.maxlen;
            //     // 注意这个int32_t要考虑 float 的情况！
            //     // 采用(*(int*)(&a))来存值
            //     std::vector<int32_t> arr (maxLen ? maxLen : 1, 0);
            //     GlobalValElement globalVar(sVarName, tp, arr, maxLen);
            //     globalValTable[sVarName] = globalVar;
            // }
            // // 构建控制流图，计算所有能够计算的全局变量值
            // const ir::Function &globalFunc = program.functions[0];
            // for (ir::Instruction* inst: globalFunc.InstVec){
            //         
            // }
    const auto &globalValList = program.globalVal;
    for (const auto& globalVal: globalValList){
        std::vector<int> ans(globalVal.maxlen ? globalVal.maxlen : 1, 0);
        backend::GlobalValElement ele(globalVal.val.name, \
                                      globalVal.val.type, \
                                      ans, \
                                      globalVal.maxlen);
        ele.draw(fout);
    }
}

void backend::Generator::get_ir_flagInfo(std::vector<ir::Instruction *>& instArr){
    // 初始化
    ir_stamp = 0;
    while(!flag_q.empty()){
        flag_q.pop();
    }
    memset(index_flag, 0, sizeof(index_flag));
    // 标注flag
    for (auto& inst: instArr){
        ir_stamp += 1;
        if (inst->op == Operator::_goto){
            int offset = std::stoi(inst->des.name);
            // 偷懒被发现了（确信）
            assert((0 <= offset + ir_stamp && offset + ir_stamp < 100000) && "Flag index exceeded!");
            index_flag[offset + ir_stamp] = goto_flag++;
        }
    }
    // queue加入flag
    for (auto& inst:instArr){
        if (inst->op == Operator::_goto){
            int offset = std::stoi(inst->des.name);
            int queryIndex = offset + ir_stamp;
            flag_q.push(index_flag[queryIndex]);
        }
    }
}

ir::Operand backend::Generator::getOperandFromStackSpace(ir::Operand opd){
    int index = memvar_Stack.size() - 1;
    assert(index - 1 >= 0 && "Invalid men stack size!");
    auto it = memvar_Stack[index].stack_table.find(opd);
    if (it != memvar_Stack[index].stack_table.end()){
        return it->first;
    }
    else{
        return Operand();
    }
}

int backend::Generator::getOffSetFromStackSpace(ir::Operand opd){
    int index = memvar_Stack.size() - 1;
    assert(index >= 0 && "Invalid men stack size!");
    auto it = memvar_Stack[index].stack_table.find(opd);
    if (it != memvar_Stack[index].stack_table.end()){
        return it->second;
    }
    else{
        assert(0 && "Inexisted stack varaiable!");
    }
}


int backend::Generator::find_operand(ir::Operand op){
    int index = memvar_Stack.size() - 1;
    assert(index >= 0 && "No Such menVar_stack size of 0!");
    return memvar_Stack[index].find_operand(op);
}

int backend::Generator::add_operand(ir::Operand op, uint32_t size){
    int index = memvar_Stack.size() - 1;
    assert(index != -1 && "No Such menVar_stack size of 0!");
    auto it = memvar_Stack[index].stack_table.find(op);
    if (it != memvar_Stack[index].stack_table.end()){       // 已经分配过了
        return memvar_Stack[index].stack_table[op];
    }
    return memvar_Stack[index].add_operand(op);
}


// 返回保存了多少个寄存器，进入前就应该保存
/* 
 * caller: 调用者保存寄存器，也就是被调用着可以随意用caller寄存器而不加以保存
 *         保存时机在调用函数前，恢复时机在调用函数后
 *         对于子程序来说，caller寄存器可以随意用，只要不调用其他函数的话。但是要记住自己用
 *         了哪些，调用函数的时候要保存。
 * callee：被调用者保存寄存器，被调用者一旦使用，要先保存再用，然后在退出的时候要还回来
 *         保存时机在进入函数的开头，恢复时机在退出函数前
 *         这一类寄存器子程序不能随便用，如果要用，要先保存再用，典型代表为sp和fp(s0)，至于
 *         其他的save reg，用的时候先保存再用，返回的时候一定要恢复！
 * 因此，我们可以总结一个策略：当前（退出或刚进入一个函数时都要清零）函数每个寄存器有个标志位，
 * 对于：
 *      caller寄存器：在使用时置flag，在调用函数时查看flag并保存，恢复时查看flag并恢复
 *      callee寄存器：在使用时置flag，置位的时候先要有store动作，退出函数前查看flag并load回
 *      寄存器
 *      对于任何寄存器，当前函数没用过就不要管，特殊的是sp，基本上每个函数都会动，特别关注
 * 
 * **具体实现时**：我们的策略是，每当一个新变量进来时，就同时在栈空间给其分配一片空间，所以不需
 *                要另外申请空间。
 */
int backend::Generator::calleeRegisterSave(){
    // 从低到高为X0到X31，由于程序是完全可控的，callee是你自己在管，所以随便用？
    // 注意sp我们不予保存
    // const unsigned int i_calleeRegisterMask = 0b00001111111111000000001100000100;
    const unsigned int i_calleeRegisterMask = 0b00001111111111000000001100000000;
    const unsigned int f_calleeRegisterMask = 0b00001111111111000000001100000000;
    // 注意保存的变量需要加入stack里面，占位置要反应偏移量的，格式用[]括着
    // 处理整数寄存器
    // fp怎么存？你所有需要存的都是要基于改变后的fp？
    // 考虑刚进入函数时，sp和fp还是上个函数的阶段，那么，我们可以先基于sp存
    // 存完后改变sp和fp(交给gen_func函数解决，其实在这里就可以解决fp，在gen_func解决sp)
    // 退出时，先把sp移动到fp处
    // 然后再基于sp来存储
    // 完美~
    for (int i = 0; i < 32; i++){
        if ((i_calleeRegisterMask >> i) & 1){
            rv::rv_inst rvInst;
            rv::rvREG saveReg = rv::rvREG(i);
            rvInst.op = rv::rvOPCODE::SW;
            rvInst.rs1 = rv::rvREG::X2;     // sp
            rvInst.rs2 = saveReg;
            Operand saveOpd;
            saveOpd.name = "[" + toGenerateString(saveReg) + "]";
            rvInst.imm = add_operand(saveOpd);
            fout << rvInst.draw();
        }
    }
    // 处理浮点数寄存器
    for (int i = 0; i < 32; i++){
        if ((f_calleeRegisterMask >> i) & 1){
            rv::rvFREG saveReg = rv::rvFREG(i);
            rv::rv_inst rvInst;
            rvInst.op = rv::rvOPCODE::FSW;
            rvInst.rs1 = rv::rvREG::X2;     // sp
            rvInst.frs2 = saveReg;
            Operand saveOpd;
            saveOpd.name = "[" + toGenerateString(saveReg) + "]";
            rvInst.imm = add_operand(saveOpd);
            fout << rvInst.draw();
        }
    }
    // 处理fp
    rv::rv_inst rvInst;
    rvInst.op = rv::rvOPCODE::MOV;
    rvInst.rd = rv::rvREG::X8;      // fp, 当然是sp的值给fp，才有fp到sp的效果
    rvInst.rs1 = rv::rvREG::X2;     // sp
    fout << rvInst.draw();
    return 24;      // 保存了24个寄存器
}

int backend::Generator::calleeRegisterRestore(){
    // 注意sp我们不予保存，sp用addi来计算，在gen_func里面管理
    // const unsigned int i_calleeRegisterMask = 0b00001111111111000000001100000100;
    const unsigned int i_calleeRegisterMask = 0b00001111111111000000001100000000;
    const unsigned int f_calleeRegisterMask = 0b00001111111111000000001100000000;
    for (int i = 0; i < 32; i++){
        if ((i_calleeRegisterMask >> i) & 1){
            rv::rv_inst rvInst;
            rv::rvREG restoreReg = rv::rvREG(i);
            rvInst.op = rv::rvOPCODE::LW;
            rvInst.rs1 = rv::rvREG::X2;     // sp
            rvInst.rd = restoreReg;
            Operand saveOpd;
            saveOpd.name = "[" + toGenerateString(restoreReg) + "]";
            rvInst.imm = find_operand(saveOpd);
            fout << rvInst.draw();
        }
    }
    // 处理浮点数寄存器
    for (int i = 0; i < 32; i++){
        if ((f_calleeRegisterMask >> i) & 1){
            rv::rvFREG restoreReg = rv::rvFREG(i);
            rv::rv_inst rvInst;
            rvInst.op = rv::rvOPCODE::FLW;
            rvInst.rs1 = rv::rvREG::X2;     // sp
            rvInst.frd = restoreReg;
            Operand saveOpd;
            saveOpd.name = "[" + toGenerateString(restoreReg) + "]";
            rvInst.imm = find_operand(saveOpd);
            fout << rvInst.draw();
        }
    }
    // 注意sp在此处的读取过程中被恢复，所以不需要另外生成riscv来恢复，退出的最后要恢复sp别忘了
    return 24;   // 恢复了24个寄存器
}

// 想一下caller保存和恢复的场景：就在处理call的IR指令时发生，所以上下文信息都不需要变动！
// 所有要存的变量，在加载进寄存器的时候都保存了！所以不需要另外开空间，直接退回到相应地方就行
int backend::Generator::callerRegisterSave(){
    const unsigned int i_callerRegisterMask = 0b11110000000000111111110011100010;
    const unsigned int f_callerRegisterMask = 0b00001111111111000000001100000000;
    int regSaveCount = 0;
    for (int i = 0; i < 32; i++){
        if (((i_callerRegisterMask >> i) & 1) && ((i_validReg >> i) & 1)){
            regSaveCount++;
            rv::rv_inst rvInst;
            rv::rvREG saveReg = rv::rvREG(i);
            rvInst.op = rv::rvOPCODE::SW;
            rvInst.rs1 = rv::rvREG::X8;         // fp
            rvInst.rs2 = saveReg;
            ir::Operand opd = i_reg2opdTable[saveReg];
            rvInst.imm = find_operand(opd);
            fout << rvInst.draw();
        }
    }
    // 处理浮点数寄存器
    for (int i = 0; i < 32; i++){
        if (((f_callerRegisterMask >> i) & 1) && ((f_validReg >> i) & 1)){
            regSaveCount++;
            rv::rv_inst rvInst;
            rv::rvFREG saveReg = rv::rvFREG(i);
            rvInst.op = rv::rvOPCODE::FSW;
            rvInst.rs1 = rv::rvREG::X8;
            rvInst.frs2 = saveReg;
            ir::Operand opd = f_reg2opdTable[saveReg];
            rvInst.imm = find_operand(opd);
            fout << rvInst.draw();
        }
    }
    return regSaveCount;
}

int backend::Generator::callerRegisterRestore(){
    const unsigned int i_callerRegisterMask = 0b11110000000000111111110011100010;
    const unsigned int f_callerRegisterMask = 0b00001111111111000000001100000000;
    int restoreRegCount = 0;
    for (int i = 0; i < 32; i++){
        if (((i_callerRegisterMask >> i) & 1) && ((i_validReg >> i) & 1)){
            // call指令
            restoreRegCount++;
            rv::rv_inst rvInst;
            rv::rvREG restoreReg = rv::rvREG(i);
            rvInst.op = rv::rvOPCODE::LW;
            rvInst.rs1 = rv::rvREG::X8;     // fp
            rvInst.rd = restoreReg;
            ir::Operand opd = i_reg2opdTable[restoreReg];
            rvInst.imm = find_operand(opd);
            fout << rvInst.draw();
        }
    }
    // 处理浮点数寄存器
    for (int i = 0; i < 32; i++){
        if (((f_callerRegisterMask >> i) & 1) && ((f_validReg >> i) & 1)){
            restoreRegCount++;
            rv::rv_inst rvInst;
            rv::rvFREG restoreReg = rv::rvFREG(i);
            rvInst.op = rv::rvOPCODE::FLW;
            rvInst.rs1 = rv::rvREG::X8;
            rvInst.frd = restoreReg;
            ir::Operand opd = f_reg2opdTable[restoreReg];
            rvInst.imm = find_operand(opd);
            fout << rvInst.draw();
        }
    }
    return restoreRegCount;
}

int backend::Generator::getStackSpaceSize(std::vector<ir::Instruction *> & func){
    std::set<ir::Operand> varSet;
    for (auto& i: func){
        varSet.insert(i->des);
        varSet.insert(i->op1);
        varSet.insert(i->op2);
    }
    return varSet.size() * 4;
}


backend::GlobalValElement::GlobalValElement(std::string _sVarName, ir::Type _tp, std::vector<int32_t> _arr, int _max){
    sVarName = _sVarName;
    tp = _tp;
    initArr = _arr;
    maxLen = _max;
    assert(int(initArr.size()) == (maxLen ? maxLen : 1) && "Unmatched initial array length");
}
// 重载赋值拷贝函数
backend::GlobalValElement backend::GlobalValElement::operator=(const GlobalValElement& ele){
    this->sVarName = ele.sVarName;
    this->tp = ele.tp;
    this->initArr = ele.initArr;
    this->maxLen = ele.maxLen;
    backend::GlobalValElement obj;
    obj.sVarName = ele.sVarName;
    obj.tp = ele.tp;
    obj.initArr = ele.initArr;
    obj.maxLen = ele.maxLen;
    return obj;
}
// 拷贝构造函数
backend::GlobalValElement::GlobalValElement(const GlobalValElement& ele){
    sVarName = ele.sVarName;
    tp = ele.tp;
    initArr = ele.initArr;
    maxLen = ele.maxLen;
}

// 输出全局变量内容
void backend::GlobalValElement::draw(std::ofstream& fout){
    if (tp == Type::IntPtr || tp==Type::FloatPtr){  // 数组
        fout << "\t.globl\t" << sVarName << endl;
        fout << "\t.align\t" << "2"      << endl;
        fout << "\t.size\t"  << sVarName << ", " <<maxLen * 4<< endl;
        fout << sVarName     << ":"      << endl;
        // // 后期可能的优化
        // int zeroCount = 0;
        // // 计算可以 .zero 初始化的段
        // for (int i = (int)initArr.size() - 1; i >= 0; i--){
        //     if (initArr[0] == 0){
        //         zeroCount++;
        //     }
        //     else{
        //         break;
        //     }
        // }
        // int wordInitCount = (int)initArr.size() - zeroCount;
        // for (int i = 0; i < wordInitCount; i++){
        //     void *numberPtr = &initArr[i];
        //     fout << "\t.word\t" << (*(int*)(numberPtr)) << endl;
        // }
        int zeroCount = maxLen;
        fout << "\t.zero\t" << zeroCount * 4 << endl;
    }
    else if(tp != Type::null){       // 变量
        fout << "\t.globl\t" << sVarName << endl;
        fout << "\t.align\t" << "2"      << endl;
        fout << "\t.type\t"  << sVarName << ", @object"     << endl;
        fout << "\t.size\t"  << sVarName << ", 4"           << endl;
        fout << sVarName     << ":"      << endl;
        if (initArr[0] == 0){
            fout << "\t.zero\t" << 4 << endl;
        }
        else{
            void *numberPtr = &initArr[0];
            fout << "\t.word\t" << (*(int*)(numberPtr)) << endl;
        }
    }
    else{
        assert(0 && "Unexpected Type!");
    }
}

// 11111210
// 0
// 0 2
// 1 5
// 2 2
// 3 2
// 4 2