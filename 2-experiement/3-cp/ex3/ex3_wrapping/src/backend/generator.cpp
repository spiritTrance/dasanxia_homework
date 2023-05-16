#include "backend/generator.h"
#include "backend/rv_def.h"
#include "backend/rv_inst_impl.h"
#include <iostream>
#include <assert.h>
#define TODO assert(0 && "todo");
#define endl "\n"

using namespace ir;
using std::cout;

backend::Generator::Generator(ir::Program& p, std::ofstream& f): program(p), fout(f) {}

int backend::stackVarMap::find_operand(ir::Operand op){
    return stack_table[op];
}

int backend::stackVarMap::add_operand(ir::Operand op, uint32_t size){
    // 感觉可以全用fp算偏移，然后sp看怎么弄
    assert(size % 4 != 0 && "Size should be the multiple of 4");
    int currSize = stack_table.size() * 4;
    int totSize = currSize + size;
    stack_table[op] = totSize;
    return totSize;
}

bool backend::Generator::isNewOperand(ir::Operand op){
    return !isInReg(op) && !isInStack(op);
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

rv::rvREG  backend::Generator::getRd(ir::Operand op){
    assert(((op.type != Type::Float) && (op.type != Type::FloatLiteral) && (op.type != Type::FloatPtr)) && "Invalid float type detected.");
    static const std::vector<rv::rvREG> tempRegList{rv::rvREG::X5, rv::rvREG::X6, rv::rvREG::X7};
    if (isInReg(op)){
        return i_opd2regTable[op];
    }
    else if(isInStack(op)){     // 在栈中
        for (rv::rvREG reg: tempRegList){
            if (!(i_imAtomicComp >> int(reg) & 1)){     // 这个不是当前计算要用的寄存器
                if ((i_validReg >> int(reg)) & 1){      // 这个寄存器正在被使用
                    Operand saveOperand = i_reg2opdTable[reg];
                    Operand loadOperand = getOperandFromStackSpace(op);
                    int offset = getOffSetFromStackSpace(op);
                    // 写回到寄存器
                    rv::rv_inst rv_save_Inst;
                    rv_save_Inst.op = rv::rvOPCODE::SW;
                    rv_save_Inst.rs2 = reg;
                    rv_save_Inst.rs1 = rv::rvREG::X8;
                    rv_save_Inst.draw();
                    // 读取进来
                    rv::rv_inst rv_load_inst;
                    rv_load_inst.op = rv::rvOPCODE::LW;
                    rv_load_inst.rd = reg;
                    rv_load_inst.rs1 = rv::rvREG::X8;
                    rv_load_inst.draw();
                }
                // 更改寄存器表信息
                i_opd2regTable[op] = reg;
                i_reg2opdTable[reg] = op;
                // 更新标志
                i_validReg |= (1 << int(reg));
                i_imAtomicComp |= (1 << int(reg));
                return reg;
            }
        }
    }
    else{       // 都不在，全新的
        for (rv::rvREG reg: tempRegList){
            if (!(i_imAtomicComp >> int(reg) & 1)){
                if ((i_validReg >> int(reg)) & 1){      // 这个寄存器正在被使用
                    Operand saveOperand = i_reg2opdTable[reg];
                    Operand loadOperand = getOperandFromStackSpace(op);
                    int offset = getOffSetFromStackSpace(op);
                    // 写回到寄存器
                    rv::rv_inst rv_save_Inst;
                    rv_save_Inst.op = rv::rvOPCODE::SW;
                    rv_save_Inst.rs2 = reg;
                    rv_save_Inst.rs1 = rv::rvREG::X8;
                    rv_save_Inst.draw();
                    // 在栈中为其分配一个位置
                    add_operand(op);
                }
                // 更改寄存器表信息
                i_opd2regTable[op] = reg;
                i_reg2opdTable[reg] = op;
                // 更新标志
                i_validReg |= (1 << int(reg));
                i_imAtomicComp |= (1 << int(reg));
                return reg;
            }
        }
    }
}

rv::rvREG  backend::Generator::getRs1(ir::Operand op){
    assert(((op.type != Type::Float) && (op.type != Type::FloatLiteral) && (op.type != Type::FloatPtr)) && "Invalid float type detected.");
    static const std::vector<rv::rvREG> tempRegList{rv::rvREG::X28, rv::rvREG::X29};
    if (isInReg(op)){
        return i_opd2regTable[op];
    }
    else if(isInStack(op)){     // 在栈中
        for (rv::rvREG reg: tempRegList){
            if (!(i_imAtomicComp >> int(reg) & 1)){     // 这个不是当前计算要用的寄存器
                if ((i_validReg >> int(reg)) & 1){      // 这个寄存器正在被使用
                    Operand saveOperand = i_reg2opdTable[reg];
                    Operand loadOperand = getOperandFromStackSpace(op);
                    int offset = getOffSetFromStackSpace(op);
                    // 写回到寄存器
                    rv::rv_inst rv_save_Inst;
                    rv_save_Inst.op = rv::rvOPCODE::SW;
                    rv_save_Inst.rs2 = reg;
                    rv_save_Inst.rs1 = rv::rvREG::X8;
                    rv_save_Inst.draw();
                    // 读取进来
                    rv::rv_inst rv_load_inst;
                    rv_load_inst.op = rv::rvOPCODE::LW;
                    rv_load_inst.rd = reg;
                    rv_load_inst.rs1 = rv::rvREG::X8;
                    rv_load_inst.draw();
                }
                // 更改寄存器表信息
                i_opd2regTable[op] = reg;
                i_reg2opdTable[reg] = op;
                // 更新标志
                i_validReg |= (1 << int(reg));
                i_imAtomicComp |= (1 << int(reg));
                return reg;
            }
        }
    }
    else{       // 都不在，全新的
        for (rv::rvREG reg: tempRegList){
            if (!(i_imAtomicComp >> int(reg) & 1)){
                if ((i_validReg >> int(reg)) & 1){      // 这个寄存器正在被使用
                    Operand saveOperand = i_reg2opdTable[reg];
                    Operand loadOperand = getOperandFromStackSpace(op);
                    int offset = getOffSetFromStackSpace(op);
                    // 写回到寄存器
                    rv::rv_inst rv_save_Inst;
                    rv_save_Inst.op = rv::rvOPCODE::SW;
                    rv_save_Inst.rs2 = reg;
                    rv_save_Inst.rs1 = rv::rvREG::X8;
                    rv_save_Inst.draw();
                    // 在栈中为其分配一个位置
                    add_operand(op);
                }
                
                // 更改寄存器表信息
                i_opd2regTable[op] = reg;
                i_reg2opdTable[reg] = op;
                // 更新标志
                i_validReg |= (1 << int(reg));
                i_imAtomicComp |= (1 << int(reg));
                return reg;
            }
        }
    }
}

rv::rvREG  backend::Generator::getRs2(ir::Operand op){
    assert(((op.type != Type::Float) && (op.type != Type::FloatLiteral) && (op.type != Type::FloatPtr)) && "Invalid float type detected.");
    static const std::vector<rv::rvREG> tempRegList{rv::rvREG::X30, rv::rvREG::X31};
    if (isInReg(op)){
        return i_opd2regTable[op];
    }
    else if(isInStack(op)){     // 在栈中
        for (rv::rvREG reg: tempRegList){
            if (!(i_imAtomicComp >> int(reg) & 1)){     // 这个不是当前计算要用的寄存器
                if ((i_validReg >> int(reg)) & 1){      // 这个寄存器正在被使用
                    Operand saveOperand = i_reg2opdTable[reg];
                    Operand loadOperand = getOperandFromStackSpace(op);
                    int offset = getOffSetFromStackSpace(op);
                    // 写回到寄存器
                    rv::rv_inst rv_save_Inst;
                    rv_save_Inst.op = rv::rvOPCODE::SW;
                    rv_save_Inst.rs2 = reg;
                    rv_save_Inst.rs1 = rv::rvREG::X8;
                    rv_save_Inst.draw();
                    // 读取进来
                    rv::rv_inst rv_load_inst;
                    rv_load_inst.op = rv::rvOPCODE::LW;
                    rv_load_inst.rd = reg;
                    rv_load_inst.rs1 = rv::rvREG::X8;
                    rv_load_inst.draw();
                }
                // 更改寄存器表信息
                i_opd2regTable[op] = reg;
                i_reg2opdTable[reg] = op;
                // 更新标志
                i_validReg |= (1 << int(reg));
                i_imAtomicComp |= (1 << int(reg));
                return reg;
            }
        }
    }
    else{       // 都不在，全新的
        for (rv::rvREG reg: tempRegList){
            if (!(i_imAtomicComp >> int(reg) & 1)){
                if ((i_validReg >> int(reg)) & 1){      // 这个寄存器正在被使用
                    Operand saveOperand = i_reg2opdTable[reg];
                    Operand loadOperand = getOperandFromStackSpace(op);
                    int offset = getOffSetFromStackSpace(op);
                    // 写回到寄存器
                    rv::rv_inst rv_save_Inst;
                    rv_save_Inst.op = rv::rvOPCODE::SW;
                    rv_save_Inst.rs2 = reg;
                    rv_save_Inst.rs1 = rv::rvREG::X8;
                    rv_save_Inst.draw();
                    // 在栈中为其分配一个位置
                    add_operand(op);
                }
                // 更改寄存器表信息
                i_opd2regTable[op] = reg;
                i_reg2opdTable[reg] = op;
                // 更新标志
                i_validReg |= (1 << int(reg));
                i_imAtomicComp |= (1 << int(reg));
                return reg;
            }
        }
    }
}

rv::rvFREG backend::Generator::fgetRd(ir::Operand op){
    assert(((op.type != Type::Int) && (op.type != Type::IntLiteral) && (op.type != Type::IntPtr)) && "Invalid float type detected.");
    static const std::vector<rv::rvFREG> tempRegList{rv::rvFREG::F0, rv::rvFREG::F1, rv::rvFREG::F2, rv::rvFREG::F3};
    if (isInReg(op)){
        return f_opd2regTable[op];
    }
    else if(isInStack(op)){     // 在栈中
        for (rv::rvFREG reg: tempRegList){
            if (!(f_imAtomicComp >> int(reg) & 1)){     // 这个不是当前计算要用的寄存器
                if ((f_validReg >> int(reg)) & 1){      // 这个寄存器正在被使用
                    Operand saveOperand = f_reg2opdTable[reg];
                    Operand loadOperand = getOperandFromStackSpace(op);
                    int offset = getOffSetFromStackSpace(op);
                    // 写回到寄存器
                    rv::rv_inst rv_save_Inst;
                    rv_save_Inst.op = rv::rvOPCODE::FSW;
                    rv_save_Inst.frs2 = reg;
                    rv_save_Inst.rs1 = rv::rvREG::X8;
                    rv_save_Inst.draw();
                    // 读取进来
                    rv::rv_inst rv_load_inst;
                    rv_load_inst.op = rv::rvOPCODE::FLW;
                    rv_load_inst.frd = reg;
                    rv_load_inst.rs1 = rv::rvREG::X8;
                    rv_load_inst.draw();
                }
                // 更改寄存器表信息
                f_opd2regTable[op] = reg;
                f_reg2opdTable[reg] = op;
                // 更改寄存器使用标志
                f_validReg |= (1 << int(reg));
                f_imAtomicComp |= (1 << int(reg));
                return reg;
            }
        }
    }
    else{       // 都不在，全新的
        for (rv::rvFREG reg: tempRegList){
            if (!(f_imAtomicComp >> int(reg) & 1)){     // 这个不是当前计算要用的寄存器
                if ((f_validReg >> int(reg)) & 1){      // 这个寄存器正在被使用
                    Operand saveOperand = f_reg2opdTable[reg];
                    Operand loadOperand = getOperandFromStackSpace(op);
                    int offset = getOffSetFromStackSpace(op);
                    // 写回到寄存器
                    rv::rv_inst rv_save_Inst;
                    rv_save_Inst.op = rv::rvOPCODE::FSW;
                    rv_save_Inst.frs2 = reg;
                    rv_save_Inst.rs1 = rv::rvREG::X8;
                    rv_save_Inst.draw();
                }
                // 更改寄存器表信息
                f_opd2regTable[op] = reg;
                f_reg2opdTable[reg] = op;
                // 更改寄存器使用标志
                f_validReg |= (1 << int(reg));
                f_imAtomicComp |= (1 << int(reg));
                return reg;
            }
        }
    }
}

rv::rvFREG backend::Generator::fgetRs1(ir::Operand op){
    assert(((op.type != Type::Int) && (op.type != Type::IntLiteral) && (op.type != Type::IntPtr)) && "Invalid float type detected.");
    static const std::vector<rv::rvFREG> tempRegList{rv::rvFREG::F4, rv::rvFREG::F5, rv::rvFREG::F6, rv::rvFREG::F7};
    if (isInReg(op)){
        return f_opd2regTable[op];
    }
    else if(isInStack(op)){     // 在栈中
        for (rv::rvFREG reg: tempRegList){
            if (!(f_imAtomicComp >> int(reg) & 1)){     // 这个不是当前计算要用的寄存器
                if ((f_validReg >> int(reg)) & 1){      // 这个寄存器正在被使用
                    Operand saveOperand = f_reg2opdTable[reg];
                    Operand loadOperand = getOperandFromStackSpace(op);
                    int offset = getOffSetFromStackSpace(op);
                    // 写回到寄存器
                    rv::rv_inst rv_save_Inst;
                    rv_save_Inst.op = rv::rvOPCODE::FSW;
                    rv_save_Inst.frs2 = reg;
                    rv_save_Inst.rs1 = rv::rvREG::X8;
                    rv_save_Inst.draw();
                    // 读取进来
                    rv::rv_inst rv_load_inst;
                    rv_load_inst.op = rv::rvOPCODE::FLW;
                    rv_load_inst.frd = reg;
                    rv_load_inst.rs1 = rv::rvREG::X8;
                    rv_load_inst.draw();
                }
                // 更改寄存器表信息
                f_opd2regTable[op] = reg;
                f_reg2opdTable[reg] = op;
                // 更改寄存器使用标志
                f_validReg |= (1 << int(reg));
                f_imAtomicComp |= (1 << int(reg));
                return reg;
            }
        }
    }
    else{       // 都不在，全新的
        for (rv::rvFREG reg: tempRegList){
            if (!(f_imAtomicComp >> int(reg) & 1)){     // 这个不是当前计算要用的寄存器
                if ((f_validReg >> int(reg)) & 1){      // 这个寄存器正在被使用
                    Operand saveOperand = f_reg2opdTable[reg];
                    Operand loadOperand = getOperandFromStackSpace(op);
                    int offset = getOffSetFromStackSpace(op);
                    // 写回到寄存器
                    rv::rv_inst rv_save_Inst;
                    rv_save_Inst.op = rv::rvOPCODE::FSW;
                    rv_save_Inst.frs2 = reg;
                    rv_save_Inst.rs1 = rv::rvREG::X8;
                    rv_save_Inst.draw();
                }
                // 更改寄存器表信息
                f_opd2regTable[op] = reg;
                f_reg2opdTable[reg] = op;
                // 更改寄存器使用标志
                f_validReg |= (1 << int(reg));
                f_imAtomicComp |= (1 << int(reg));
                return reg;
            }
        }
    }
}

rv::rvFREG backend::Generator::fgetRs2(ir::Operand op){
    assert(((op.type != Type::Int) && (op.type != Type::IntLiteral) && (op.type != Type::IntPtr)) && "Invalid float type detected.");
    static const std::vector<rv::rvFREG> tempRegList{rv::rvFREG::F28, rv::rvFREG::F29, rv::rvFREG::F30, rv::rvFREG::F31};
    if (isInReg(op)){
        return f_opd2regTable[op];
    }
    else if(isInStack(op)){     // 在栈中
        for (rv::rvFREG reg: tempRegList){
            if (!(f_imAtomicComp >> int(reg) & 1)){     // 这个不是当前计算要用的寄存器
                if ((f_validReg >> int(reg)) & 1){      // 这个寄存器正在被使用
                    Operand saveOperand = f_reg2opdTable[reg];
                    Operand loadOperand = getOperandFromStackSpace(op);
                    int offset = getOffSetFromStackSpace(op);
                    // 写回到寄存器
                    rv::rv_inst rv_save_Inst;
                    rv_save_Inst.op = rv::rvOPCODE::FSW;
                    rv_save_Inst.frs2 = reg;
                    rv_save_Inst.rs1 = rv::rvREG::X8;
                    rv_save_Inst.draw();
                    // 读取进来
                    rv::rv_inst rv_load_inst;
                    rv_load_inst.op = rv::rvOPCODE::FLW;
                    rv_load_inst.frd = reg;
                    rv_load_inst.rs1 = rv::rvREG::X8;
                    rv_load_inst.draw();
                }
                // 更改寄存器表信息
                f_opd2regTable[op] = reg;
                f_reg2opdTable[reg] = op;
                // 更改寄存器使用标志
                f_validReg |= (1 << int(reg));
                f_imAtomicComp |= (1 << int(reg));
                return reg;
            }
        }
    }
    else{       // 都不在，全新的
        for (rv::rvFREG reg: tempRegList){
            if (!(f_imAtomicComp >> int(reg) & 1)){     // 这个不是当前计算要用的寄存器
                if ((f_validReg >> int(reg)) & 1){      // 这个寄存器正在被使用
                    Operand saveOperand = f_reg2opdTable[reg];
                    Operand loadOperand = getOperandFromStackSpace(op);
                    int offset = getOffSetFromStackSpace(op);
                    // 写回到寄存器
                    rv::rv_inst rv_save_Inst;
                    rv_save_Inst.op = rv::rvOPCODE::FSW;
                    rv_save_Inst.frs2 = reg;
                    rv_save_Inst.rs1 = rv::rvREG::X8;
                    rv_save_Inst.draw();
                }
                // 更改寄存器表信息
                f_opd2regTable[op] = reg;
                f_reg2opdTable[reg] = op;
                // 更改寄存器使用标志
                f_validReg |= (1 << int(reg));
                f_imAtomicComp |= (1 << int(reg));
                return reg;
            }
        }
    }
}


// generate wrapper function
// 注意function那个数组的第一个一定是全局变量函数
void backend::Generator::gen() {
    backend::Generator::gen_globalVal();
}

void backend::Generator::gen_func(ir::Function& func){
    // 每进入一个块，寄存器caller保存寄存器有效位清空
    fout << func.name << ":"<<endl;     // 标签标注
    // 被调用者保存寄存器
    TODO;
    for (auto inst: func.InstVec){
        gen_instr(*inst);
    }
}

void backend::Generator::gen_instr(ir::Instruction& inst){
    // 库函数
    static std::map<std::string,ir::Function*> lib_funcs = {
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
        case ir::Operator::def:     // 认为马上就会用，就放寄存器吧
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
            inst.draw();
        }
            break;
        case ir::Operator::fdef:
        case ir::Operator::fmov:
        {
            rv::rv_inst inst;
            inst.frd = fgetRd(des);
            if (op1.type == Type::FloatLiteral){
                // 小心这里丢精度
                float f = std::stof(op1.name);          
                inst.imm = (*(uint32_t*)(&f));
                inst.op = rv::rvOPCODE::FLI;
            }
            else{
                inst.frs1 = fgetRs1(op1);
                inst.op = rv::rvOPCODE::FMOV;
            }
            inst.draw();
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
            rvInst.draw();
        }
            break;
        case ir::Operator::addi:
        {
            rv::rv_inst rvInst;
            rvInst.rd = getRd(des);
            rvInst.rs1 = getRs1(op1);
            rvInst.imm = std::stoi(op2.name);
            rvInst.op = rv::rvOPCODE::ADDI;
            rvInst.draw();
        }
            break;
        case ir::Operator::fadd:
        {
            rv::rv_inst rvInst;
            // 三种情况：有一个是字面量；全是临时变量
            // 由于没有faddi，所以要迁移一下，多个li的伪指令
            // 我们约定第一个算寄存器的，第二个是立即数加载出来的
            rvInst.frd = fgetRd(des);
            rvInst.op = rv::rvOPCODE::FADD_S;
            if (op1.type == Type::FloatLiteral && op2.type == Type::FloatLiteral){
                float f1 = std::stof(op1.name);
                float f2 = std::stof(op2.name);
                int32_t u1 = *(int32_t *)(&f1);
                int32_t u2 = *(int32_t *)(&f2);
                rv::rv_inst rvInst_li;
                rvInst_li.op = rv::rvOPCODE::FLI;
                rvInst.frs1 = rvInst_li.frd = fgetRd(op1);
                rvInst_li.imm = u1;
                rvInst_li.draw();
                rvInst.frs2 = rvInst_li.frd = fgetRd(op2);
                rvInst_li.imm = u2;
                rvInst_li.draw();
            }
            else if (op1.type == Type::FloatLiteral){
                float f1 = std::stof(op1.name);
                int32_t u1 = *(int32_t *)(&f1);
                rv::rv_inst rvInst_li;
                rvInst_li.op = rv::rvOPCODE::FLI;
                rvInst.frs1 = rvInst_li.frd = fgetRd(op1);
                rvInst_li.imm = u1;
                rvInst_li.draw();
                rvInst.frs2 = fgetRs2(op2);
            }
            else if (op2.type == Type::FloatLiteral){
                float f2 = std::stof(op2.name);
                int32_t u2 = *(int32_t *)(&f2);
                rv::rv_inst rvInst_li;
                rvInst_li.op = rv::rvOPCODE::FLI;
                rvInst.frs2 = rvInst_li.frd = fgetRd(op2);
                rvInst_li.imm = u2;
                rvInst_li.draw();
                rvInst.frs1 = fgetRs2(op1);
            }
            else{
                rvInst.frs1 = fgetRs1(op1);
                rvInst.frs2 = fgetRs1(op2);
            }
            rvInst.draw();
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
                rvInst.rs1 = getRd(op1);
                rvInst.imm = -(uint32_t)std::stoi(op2.name);
                rvInst.draw();
            }
            else{
                rvInst.rs1 = getRs1(op1);
                rvInst.rs2 = getRs2(op2);
                rvInst.op = rv::rvOPCODE::SUB;
            }
            rvInst.draw();
        }
            break;
        case ir::Operator::subi:
        {
            rv::rv_inst rvInst;
            rvInst.op = rv::rvOPCODE::ADDI;
            rvInst.rd = getRd(des);
            rvInst.rs1 = getRd(op1);
            rvInst.imm = -(uint32_t)std::stoi(op2.name);
            rvInst.draw();
        }
            break;
        case ir::Operator::fsub:
        {
            rv::rv_inst rvInst;
            // 三种情况：有一个是字面量；全是临时变量
            // 由于没有faddi，所以要迁移一下，多个li的伪指令
            // 我们约定第一个算寄存器的，第二个是立即数加载出来的
            rvInst.frd = fgetRd(des);
            rvInst.op = rv::rvOPCODE::FSUB_S;
            if (op1.type == Type::FloatLiteral && op2.type == Type::FloatLiteral){
                float f1 = std::stof(op1.name);
                float f2 = std::stof(op2.name);
                int32_t u1 = *(int32_t *)(&f1);
                int32_t u2 = *(int32_t *)(&f2);
                rv::rv_inst rvInst_li;
                rvInst_li.op = rv::rvOPCODE::FLI;
                rvInst.frs1 = rvInst_li.frd = fgetRd(op1);
                rvInst_li.imm = u1;
                rvInst_li.draw();
                rvInst.frs2 = rvInst_li.frd = fgetRd(op2);
                rvInst_li.imm = u2;
                rvInst_li.draw();
            }
            else if (op1.type == Type::FloatLiteral){
                float f1 = std::stof(op1.name);
                int32_t u1 = *(int32_t *)(&f1);
                rv::rv_inst rvInst_li;
                rvInst_li.op = rv::rvOPCODE::FLI;
                rvInst.frs1 = rvInst_li.frd = fgetRd(op1);
                rvInst_li.imm = u1;
                rvInst_li.draw();
                rvInst.frs2 = fgetRs2(op2);
            }
            else if (op2.type == Type::FloatLiteral){
                float f2 = std::stof(op2.name);
                int32_t u2 = *(int32_t *)(&f2);
                rv::rv_inst rvInst_li;
                rvInst_li.op = rv::rvOPCODE::FLI;
                rvInst.frs2 = rvInst_li.frd = fgetRd(op2);
                rvInst_li.imm = u2;
                rvInst_li.draw();
                rvInst.frs1 = fgetRs2(op1);
            }
            else{
                rvInst.frs1 = fgetRs1(op1);
                rvInst.frs2 = fgetRs1(op2);
            }
            rvInst.draw();
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
                rvInst.rs1 = rvInst_li.rd = getRd(op1);
                rvInst_li.imm = u1;
                rvInst_li.draw();
                rvInst.rs2 = rvInst_li.rd = getRd(op2);
                rvInst_li.imm = u2;
                rvInst_li.draw();
            }
            else if (op1.type == Type::IntLiteral){
                int32_t u1 = std::stoi(op1.name);
                rv::rv_inst rvInst_li;
                rvInst_li.op = rv::rvOPCODE::LI;
                rvInst.rs1 = rvInst_li.rd = getRd(op1);
                rvInst_li.imm = u1;
                rvInst_li.draw();
                rvInst.rs2 = getRs2(op2);
            }
            else if (op2.type == Type::IntLiteral){
                int32_t u2 = std::stoi(op2.name);
                rv::rv_inst rvInst_li;
                rvInst_li.op = rv::rvOPCODE::LI;
                rvInst.rs2 = rvInst_li.rd = getRd(op2);
                rvInst_li.imm = u2;
                rvInst_li.draw();
                rvInst.rs1 = getRs2(op1);
            }
            else{
                rvInst.rs1 = getRs1(op1);
                rvInst.rs2 = getRs1(op2);
            }
            rvInst.draw();
        }
            break;
        case ir::Operator::fmul:
        case ir::Operator::fdiv:
        {
            rv::rv_inst rvInst;
            // 三种情况：有一个是字面量；全是临时变量
            // 由于没有faddi，所以要迁移一下，多个li的伪指令
            // 我们约定第一个算寄存器的，第二个是立即数加载出来的
            rvInst.frd = fgetRd(des);
            rvInst.op = op == Operator::fmul ? rv::rvOPCODE::FMUL_S : rv::rvOPCODE::FDIV_S;
            if (op1.type == Type::FloatLiteral && op2.type == Type::FloatLiteral){
                float f1 = std::stof(op1.name);
                float f2 = std::stof(op2.name);
                int32_t u1 = *(int32_t *)(&f1);
                int32_t u2 = *(int32_t *)(&f2);
                rv::rv_inst rvInst_li;
                rvInst_li.op = rv::rvOPCODE::FLI;
                rvInst.frs1 = rvInst_li.frd = fgetRd(op1);
                rvInst_li.imm = u1;
                rvInst_li.draw();
                rvInst.frs2 = rvInst_li.frd = fgetRd(op2);
                rvInst_li.imm = u2;
                rvInst_li.draw();
            }
            else if (op1.type == Type::FloatLiteral){
                float f1 = std::stof(op1.name);
                int32_t u1 = *(int32_t *)(&f1);
                rv::rv_inst rvInst_li;
                rvInst_li.op = rv::rvOPCODE::FLI;
                rvInst.frs1 = rvInst_li.frd = fgetRd(op1);
                rvInst_li.imm = u1;
                rvInst_li.draw();
                rvInst.frs2 = fgetRs2(op2);
            }
            else if (op2.type == Type::FloatLiteral){
                float f2 = std::stof(op2.name);
                int32_t u2 = *(int32_t *)(&f2);
                rv::rv_inst rvInst_li;
                rvInst_li.op = rv::rvOPCODE::FLI;
                rvInst.frs2 = rvInst_li.frd = fgetRd(op2);
                rvInst_li.imm = u2;
                rvInst_li.draw();
                rvInst.frs1 = fgetRs2(op1);
            }
            else{
                rvInst.frs1 = fgetRs1(op1);
                rvInst.frs2 = fgetRs1(op2);
            }
            rvInst.draw();
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
                rvInst.rs1 = rvInst_li.rd = getRd(op1);
                rvInst_li.imm = u1;
                rvInst_li.draw();
                rvInst.rs2 = rvInst_li.rd = getRd(op2);
                rvInst_li.imm = u2;
                rvInst_li.draw();
            }
            else if (op1.type == Type::IntLiteral){
                int32_t u1 = std::stoi(op1.name);
                rv::rv_inst rvInst_li;
                rvInst_li.op = rv::rvOPCODE::LI;
                rvInst.rs1 = rvInst_li.rd = getRd(op1);
                rvInst_li.imm = u1;
                rvInst_li.draw();
                rvInst.rs2 = getRs2(op2);
            }
            else if (op2.type == Type::IntLiteral){
                int32_t u2 = std::stoi(op2.name);
                rv::rv_inst rvInst_li;
                rvInst_li.op = rv::rvOPCODE::LI;
                rvInst.rs2 = rvInst_li.rd = getRd(op2);
                rvInst_li.imm = u2;
                rvInst_li.draw();
                rvInst.rs1 = getRs2(op1);
            }
            else{
                rvInst.rs1 = getRs1(op1);
                rvInst.rs2 = getRs1(op2);
            }
            rvInst.draw();
        }
            break;
        case ir::Operator::flss:    // <        FLT
        case ir::Operator::fleq:    // <=       FLE
        case ir::Operator::fgtr:    // >        not exist
        case ir::Operator::fgeq:    // >=       not exist
        case ir::Operator::feq:     // ==
        case ir::Operator::fneq:    // !=
        {
            // 注意有很多bug了！看是FLI还是LI还是啥，bug有点多，主要是状态对不上
            rv::rv_inst rvInst;
            // 四种情况：Int和Literal两两组合
            // 由于没有faddi，所以要迁移一下，多个li的伪指令
            // 我们约定第一个算寄存器的，第二个是立即数加载出来的
            rvInst.frd = fgetRd(des);
            rvInst.op = op == Operator::flss ? rv::rvOPCODE::FLT_S :\
                        op == Operator::fleq ? rv::rvOPCODE::FLE_S :\
                        op == Operator::feq  ? rv::rvOPCODE::FEQ_S :\
                        op == Operator::fneq ? rv::rvOPCODE::FNEQ_S:\
                        op == Operator::fgtr ? rv::rvOPCODE::FLT_S : rv::rvOPCODE::FLE_S;
            // 两种情况，特殊交换一下
            if (op == Operator::fgtr || op == Operator::fgeq){
                std::swap(op1, op2);
            }
            if (op1.type == Type::FloatLiteral && op2.type == Type::FloatLiteral){
                float u1 = std::stof(op1.name);
                float u2 = std::stof(op2.name);
                rv::rv_inst rvInst_li;
                rvInst_li.op = rv::rvOPCODE::FLI;
                rvInst.frs1 = rvInst_li.frd = fgetRd(op1);
                rvInst_li.imm = *(uint32_t*)(&u1);
                rvInst_li.draw();
                rvInst.frs2 = rvInst_li.frd = fgetRd(op2);
                rvInst_li.imm = *(uint32_t*)(&u2);
                rvInst_li.draw();
            }
            else if (op1.type == Type::FloatLiteral){
                float u1 = std::stof(op1.name);
                rv::rv_inst rvInst_li;
                rvInst_li.op = rv::rvOPCODE::FLI;
                rvInst.frs1 = rvInst_li.frd = fgetRd(op1);
                rvInst_li.imm = *(uint32_t*)(&u1);
                rvInst_li.draw();
                rvInst.frs2 = fgetRs2(op2);
            }
            else if (op2.type == Type::FloatLiteral){
                float u2 = std::stof(op2.name);
                rv::rv_inst rvInst_li;
                rvInst_li.op = rv::rvOPCODE::FLI;
                rvInst.frs2 = rvInst_li.frd = fgetRd(op2);
                rvInst_li.imm = *(uint32_t*)(&u2);
                rvInst_li.draw();
                rvInst.frs1 = fgetRs2(op1);
            }
            else{
                rvInst.frs1 = fgetRs1(op1);
                rvInst.frs2 = fgetRs1(op2);
            }
            rvInst.draw();
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
            rvInst.draw();
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
                rvInst.rs1 = rvInst_li.rd = getRd(op1);
                rvInst_li.imm = u1;
                rvInst_li.draw();
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
                rvInst.rs2 = getRs1(op2);
            }
            rvInst.draw();
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
            rv::rv_inst rvInst;
            if (op1.type == Type::FloatPtr){
                rvInst.op = rv::rvOPCODE::FLW;
                rvInst.frd = fgetRd(des);
                rvInst.rs1 = getRs1(op1);
                rvInst.imm = std::stoi(op2.name);
            }
            else if (op1.type == Type::IntPtr){
                rvInst.op = rv::rvOPCODE::LW;
                rvInst.rd = getRd(des);
                rvInst.rs1 = getRs1(op1);
                rvInst.imm = std::stoi(op2.name);
            }
            else{
                assert(0 && "Invalid type!");
            }
            rvInst.draw();
        }
            break;
        case ir::Operator::store:   // des 是目标存入变量，op1 是数组名，op2是偏移量
        {
            rv::rv_inst rvInst;
            if (op1.type == Type::FloatPtr){
                rvInst.op = rv::rvOPCODE::FSW;
                rvInst.frs2 = fgetRs2(des);      // 要存的值，注意一下
                rvInst.rs1 = getRs1(op1);
                rvInst.imm = std::stoi(op2.name);
            }
            else if (op1.type == Type::IntPtr){
                rvInst.op = rv::rvOPCODE::SW;
                rvInst.rs2 = getRs2(des);
                rvInst.rs1 = getRs1(op1);
                rvInst.imm = std::stoi(op2.name);
            }
            else{
                assert(0 && "Invalid type!");
            }
            rvInst.draw();
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
        {
            rv::rv_inst rvInst;
            rvInst.op = rv::rvOPCODE::LI;
            TODO;
        }
            break;
        // function control
        case ir::Operator::call:    // 难度较大，想一下sp指针如何移动？在哪移动？调用前移动好吗？显然不好。但参数如何保存得知道
        {
            ir::Instruction *instPtr = &inst;
            ir::CallInst* callInstPtr= dynamic_cast<ir::CallInst*>(instPtr);    // callInst 要向下类型转换
            assert(callInstPtr && "Not a callInst.");
        }
            TODO;
            break;
        case ir::Operator::_return:
        {
            TODO;
        }
            break;
        // _goto
        case ir::Operator::_goto:
        {
            TODO;
        }
            break;
        // convertion
        case ir::Operator::cvt_f2i:
        {
            rv::rv_inst rvInst;
            rvInst.op = rv::rvOPCODE::FCVT_W_S;
            rvInst.rd = getRd(des);
            rvInst.frs1 = fgetRd(op1);
            rvInst.draw();
        }
            break;
        case ir::Operator::cvt_i2f:
        {
            rv::rv_inst rvInst;
            rvInst.op = rv::rvOPCODE::FCVT_S_W;
            rvInst.frd = fgetRd(des);
            rvInst.rs1 = getRd(op1);
            rvInst.draw();
        }
            break;
        // unuse
        case ir::Operator::__unuse__:
        {
            rv::rv_inst rvInst;
            rvInst.op = rv::rvOPCODE::NOP;
            rvInst.draw();
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

// 全局变量函数的调用
void backend::Generator::gen_globalFunc(ir::Instruction& inst){

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
    assert(index - 1 >= 0 && "Invalid men stack size!");
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
    assert(index != -1 && "No Such menVar_stack size of 0!");
    return memvar_Stack[index].find_operand(op);
}

int backend::Generator::add_operand(ir::Operand op, uint32_t size = 4){
    int index = memvar_Stack.size() - 1;
    assert(index != -1 && "No Such menVar_stack size of 0!");
    return memvar_Stack[index].add_operand(op);
}


// 返回保存了多少个寄存器
int backend::Generator::calleeRegisterSave(){

}

int backend::Generator::callerRegisterSave(){

}

// algorithm
// std::vector<ir::Operand> backend::Generator::linearScan(const std::vector<ir::Instruction *>) const{
//     TODO;
// }

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