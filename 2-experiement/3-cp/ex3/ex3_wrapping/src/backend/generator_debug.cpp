#include "backend/rv_inst_impl.h"
#include"backend/rv_def.h"
#include "backend/generator.h"
#include <iostream>
#include <map>
#include <queue>
#include <set>
#include <assert.h>
#include <string.h>
#include <bitset>   // debug
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
    return 0;
}

int backend::stackVarMap::add_operand(ir::Operand op, int32_t size){
    return 0; // 相对于fp来算的话，fp是大地址，那么-fp是小地址，数组的话，正向偏移，和xx一致的。
}

bool backend::Generator::isNewOperand(ir::Operand op){
    return false;
}

bool backend::Generator::isGlobalVar(ir::Operand op){
    return false;
}

bool backend::Generator::isInStack(ir::Operand op){
    return false;
}

bool backend::Generator::isInReg(ir::Operand op){
    return false;
}

// caseOp = 0: int, caseOp = 1: float
void backend::Generator::expireRegData(int regIndex, int caseOp){
    return;
}       // 将寄存器里面的值移回到内存，注意可能是全局变量

void backend::Generator::loadMemData(int regIndex, ir::Operand op){
    return;
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
    return rv::rvREG::X0;
}

rv::rvREG  backend::Generator::getRs1(ir::Operand op){    return rv::rvREG::X0;
    return rv::rvREG::X0;
}

rv::rvREG  backend::Generator::getRs2(ir::Operand op){
    return rv::rvREG::X0;
}

rv::rvFREG backend::Generator::fgetRd(ir::Operand op){
    return rv::rvFREG::F0;
}

rv::rvFREG backend::Generator::fgetRs1(ir::Operand op){
    return rv::rvFREG::F0;
}

rv::rvFREG backend::Generator::fgetRs2(ir::Operand op){
    return rv::rvFREG::F0;
}

// generate wrapper function
// 注意function那个数组的第一个一定是全局变量函数
void backend::Generator::gen() {
    TODO;
}

void backend::Generator::gen_func(ir::Function& func){
    TODO;
}

void backend::Generator::gen_instr(ir::Instruction& inst){
    TODO;
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
    // 退出前的stamp要清空
    ir_stamp = 0;
}

// ir::Operand backend::Generator::getOperandFromStackSpace(ir::Operand opd){
//     int index = memvar_Stack.size() - 1;
//     assert(index >= 0 && "Invalid men stack size!");
//     std::map<ir::Operand, int>::iterator it = memvar_Stack[index].stack_table.find(opd);
//     // if (it != memvar_Stack[index].stack_table.end()){
//     //     return it->first;
//     // }
//     // else{
//         return ir::Operand();
//     // }
// }

// int backend::Generator::getOffSetFromStackSpace(ir::Operand opd){
//     int index = memvar_Stack.size() - 1;
//     assert(index >= 0 && "Invalid men stack size!");
//     auto it = memvar_Stack[index].stack_table.find(opd);
//     if (it != memvar_Stack[index].stack_table.end()){
//         return it->second;
//     }
//     else{
//         // cout << "In getOffsetFromStackSpace: " << opd.name << endl;
//         assert(0 && "Inexisted stack varaiable!");
//     }
// }


// int backend::Generator::find_operand(ir::Operand op){
//     int index = memvar_Stack.size() - 1;
//     // cout << "In find_operand: " << index << endl;
//     assert(index >= 0 && "No Such menVar_stack size of 0!");
//     return memvar_Stack[index].find_operand(op);
// }

// int backend::Generator::add_operand(ir::Operand op, int32_t size){
//     int index = memvar_Stack.size() - 1;
//     // cout << "In add_operand: " << op.name << ' '<<index<<endl;
//     assert(index >= 0 && "No Such menVar_stack size of 0!");
//     // cout << "In add_pod: " << index << endl;
//     auto it = memvar_Stack[index].stack_table.find(op);
//     if (it != memvar_Stack[index].stack_table.end()){       // 已经分配过了
//         // cout << "In Operand: Already Allocated." << endl;
//         return memvar_Stack[index].stack_table[op];
//     }
//     return memvar_Stack[index].add_operand(op);
// }


// // 返回保存了多少个寄存器，进入前就应该保存
// /* 
//  * caller: 调用者保存寄存器，也就是被调用着可以随意用caller寄存器而不加以保存
//  *         保存时机在调用函数前，恢复时机在调用函数后
//  *         对于子程序来说，caller寄存器可以随意用，只要不调用其他函数的话。但是要记住自己用
//  *         了哪些，调用函数的时候要保存。
//  * callee：被调用者保存寄存器，被调用者一旦使用，要先保存再用，然后在退出的时候要还回来
//  *         保存时机在进入函数的开头，恢复时机在退出函数前
//  *         这一类寄存器子程序不能随便用，如果要用，要先保存再用，典型代表为sp和fp(s0)，至于
//  *         其他的save reg，用的时候先保存再用，返回的时候一定要恢复！
//  * 因此，我们可以总结一个策略：当前（退出或刚进入一个函数时都要清零）函数每个寄存器有个标志位，
//  * 对于：
//  *      caller寄存器：在使用时置flag，在调用函数时查看flag并保存，恢复时查看flag并恢复
//  *      callee寄存器：在使用时置flag，置位的时候先要有store动作，退出函数前查看flag并load回
//  *      寄存器
//  *      对于任何寄存器，当前函数没用过就不要管，特殊的是sp，基本上每个函数都会动，特别关注
//  * 
//  * **具体实现时**：我们的策略是，每当一个新变量进来时，就同时在栈空间给其分配一片空间，所以不需
//  *                要另外申请空间。
//  */
// int backend::Generator::calleeRegisterSave(){
//     // 从低到高为X0到X31，由于程序是完全可控的，callee是你自己在管，所以随便用？
//     // 注意sp我们不予保存
//     // const unsigned int i_calleeRegisterMask = 0b00001111111111000000001100000100;
//     const unsigned int i_calleeRegisterMask = 0b00001111111111000000001100000010;
//     const unsigned int f_calleeRegisterMask = 0b00001111111111000000001100000000;
//     // 注意保存的变量需要加入stack里面，占位置要反应偏移量的，格式用[]括着
//     // 处理整数寄存器
//     // fp怎么存？你所有需要存的都是要基于改变后的fp？
//     // 考虑刚进入函数时，sp和fp还是上个函数的阶段，那么，我们可以先基于sp存
//     // 存完后改变sp和fp(交给gen_func函数解决，其实在这里就可以解决fp，在gen_func解决sp)
//     // 退出时，先把sp移动到fp处
//     // 然后再基于sp来存储
//     // 完美~
//     for (int i = 0; i < 32; i++){
//         if ((i_calleeRegisterMask >> i) & 1){
//             rv::rv_inst rvInst;
//             rv::rvREG saveReg = rv::rvREG(i);
//             rvInst.op = rv::rvOPCODE::SW;
//             rvInst.rs1 = rv::rvREG::X2;     // sp
//             rvInst.rs2 = saveReg;
//             Operand saveOpd;
//             saveOpd.name = "[" + toString(saveReg) + "]";
//             saveOpd.type = Type::Int;
//             rvInst.imm = add_operand(saveOpd);
//             fout << rvInst.draw();
//         }
//     }
//     // 处理浮点数寄存器
//     for (int i = 0; i < 32; i++){
//         if ((f_calleeRegisterMask >> i) & 1){
//             rv::rvFREG saveReg = rv::rvFREG(i);
//             rv::rv_inst rvInst;
//             rvInst.op = rv::rvOPCODE::FSW;
//             rvInst.rs1 = rv::rvREG::X2;     // sp
//             rvInst.frs2 = saveReg;
//             Operand saveOpd;
//             saveOpd.name = "[" + toString(saveReg) + "]";
//             saveOpd.type = Type::Float;
//             rvInst.imm = add_operand(saveOpd);
//             fout << rvInst.draw();
//         }
//     }
//     // 处理fp
//     rv::rv_inst rvInst;
//     rvInst.op = rv::rvOPCODE::MOV;
//     rvInst.rd = rv::rvREG::X8;      // fp, 当然是sp的值给fp，才有fp到sp的效果
//     rvInst.rs1 = rv::rvREG::X2;     // sp
//     fout << rvInst.draw();
//     return 24;      // 保存了24个寄存器
// }

// int backend::Generator::calleeRegisterRestore(){
//     // 注意sp我们不予保存，sp用addi来计算，在gen_func里面管理
//     // const unsigned int i_calleeRegisterMask = 0b00001111111111000000001100000100;
//     // cout << "Trigger calleeRegSave." << endl;
//     const unsigned int i_calleeRegisterMask = 0b00001111111111000000001100000010;
//     const unsigned int f_calleeRegisterMask = 0b00001111111111000000001100000000;
//     // 想一下，在函数退出的时候，要把全局变量恢复回去哦
//     for (int i = 0; i < 32; i++){
//         if ((i_calleeRegisterMask >> i) & 1){
//             rv::rv_inst rvInst;
//             rv::rvREG restoreReg = rv::rvREG(i);
//             rvInst.op = rv::rvOPCODE::LW;
//             rvInst.rs1 = rv::rvREG::X2;     // sp
//             rvInst.rd = restoreReg;
//             Operand saveOpd;
//             saveOpd.name = "[" + toString(restoreReg) + "]";
//             saveOpd.type = Type::Int;
//             rvInst.imm = find_operand(saveOpd);
//             fout << rvInst.draw();
//         }
//     }
//     // 处理浮点数寄存器
//     for (int i = 0; i < 32; i++){
//         if ((f_calleeRegisterMask >> i) & 1){
//             rv::rvFREG restoreReg = rv::rvFREG(i);
//             rv::rv_inst rvInst;
//             rvInst.op = rv::rvOPCODE::FLW;
//             rvInst.rs1 = rv::rvREG::X2;     // sp
//             rvInst.frd = restoreReg;
//             Operand saveOpd;
//             saveOpd.name = "[" + toString(restoreReg) + "]";
//             saveOpd.type = Type::Float;
//             rvInst.imm = find_operand(saveOpd);
//             fout << rvInst.draw();
//         }
//     }
//     // 注意sp在此处的读取过程中被恢复，所以不需要另外生成riscv来恢复，退出的最后要恢复sp别忘了
//     return 24;   // 恢复了24个寄存器
// }

// // 想一下caller保存和恢复的场景：就在处理call的IR指令时发生，所以上下文信息都不需要变动！
// // 所有要存的变量，在加载进寄存器的时候都保存了！所以不需要另外开空间，直接退回到相应地方就行
// int backend::Generator::callerRegisterSave(){
//     const unsigned int i_callerRegisterMask = 0b11110000000000111111110011100010;
//     const unsigned int f_callerRegisterMask = 0b00001111111111000000001100000000;
//     int regSaveCount = 0;
//     for (int i = 0; i < 32; i++){
//         if (((i_callerRegisterMask >> i) & 1) && ((i_validReg >> i) & 1)){
//             regSaveCount++;
//             rv::rv_inst rvInst;
//             rv::rvREG saveReg = rv::rvREG(i);
//             ir::Operand opd = i_reg2opdTable[saveReg];
//             if (isGlobalVar(opd)){      // 全局变量
//                 fout<<"\t"<<"lui\t"<<toString(rv::rvREG::X7)<<","<<"\%hi("<<opd.name<<")"<<endl;
//                 fout<<"\t"<<"sw\t"<<toString(saveReg)<<","<<"\%lo("<<opd.name<<")("<<toString(rv::rvREG::X7)<<")"<<endl;
//             }
//             else{
//                 rvInst.op = rv::rvOPCODE::SW;
//                 rvInst.rs1 = rv::rvREG::X8;         // fp
//                 rvInst.rs2 = saveReg;
//                 rvInst.imm = find_operand(opd);
//                 fout << rvInst.draw();
//             }
//         }
//     }
//     // 处理浮点数寄存器
//     for (int i = 0; i < 32; i++){
//         if (((f_callerRegisterMask >> i) & 1) && ((f_validReg >> i) & 1)){
//             regSaveCount++;
//             rv::rv_inst rvInst;
//             rv::rvFREG saveReg = rv::rvFREG(i);
//             ir::Operand opd = f_reg2opdTable[saveReg];
//             if (isGlobalVar(opd)){      // 全局变量
//                 fout<<"\t"<<"lui\t"<<toString(rv::rvREG::X7)<<","<<"\%hi("<<opd.name<<")"<<endl;
//                 fout<<"\t"<<"fsw\t"<<toString(saveReg)<<","<<"\%lo("<<opd.name<<")("<<toString(rv::rvREG::X7)<<")"<<endl;
//             }
//             else{
//                 rvInst.op = rv::rvOPCODE::FSW;
//                 rvInst.rs1 = rv::rvREG::X8;
//                 rvInst.frs2 = saveReg;
//                 rvInst.imm = find_operand(opd);
//                 fout << rvInst.draw();
//             }
//         }
//     }
//     return regSaveCount;
// }

// int backend::Generator::callerRegisterRestore(){
//     const unsigned int i_callerRegisterMask = 0b11110000000000111111110011100010;
//     const unsigned int f_callerRegisterMask = 0b00001111111111000000001100000000;
//     int restoreRegCount = 0;
//     for (int i = 0; i < 32; i++){
//         if (((i_callerRegisterMask >> i) & 1) && ((i_validReg >> i) & 1)){
//             // call指令
//             restoreRegCount++;
//             rv::rv_inst rvInst;
//             rv::rvREG restoreReg = rv::rvREG(i);
//             rvInst.op = rv::rvOPCODE::LW;
//             rvInst.rs1 = rv::rvREG::X8;     // fp
//             rvInst.rd = restoreReg;
//             ir::Operand opd = i_reg2opdTable[restoreReg];
//             rvInst.imm = find_operand(opd);
//             fout << rvInst.draw();
//         }
//     }
//     // 处理浮点数寄存器
//     for (int i = 0; i < 32; i++){
//         if (((f_callerRegisterMask >> i) & 1) && ((f_validReg >> i) & 1)){
//             restoreRegCount++;
//             rv::rv_inst rvInst;
//             rv::rvFREG restoreReg = rv::rvFREG(i);
//             rvInst.op = rv::rvOPCODE::FLW;
//             rvInst.rs1 = rv::rvREG::X8;
//             rvInst.frd = restoreReg;
//             ir::Operand opd = f_reg2opdTable[restoreReg];
//             rvInst.imm = find_operand(opd);
//             fout << rvInst.draw();
//         }
//     }
//     return restoreRegCount;
// }

int backend::Generator::getStackSpaceSize(std::vector<ir::Instruction *> & func){
    std::set<ir::Operand> varSet;
    for (auto i : func){
        // varSet.insert(i->des);
        // varSet.insert(i->op1);
        // varSet.insert(i->op2);
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
        fout << "\t.section\t" << ".sdata,\"aw\"" << endl;
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
        fout << "\t.section\t" << ".sdata,\"aw\"" << endl;
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

// // 11111210
// // 0
// // 0 2
// // 1 5
// // 2 2
// // 3 2
// // 4 2


// impl
std::string rv::rv_inst::draw() const{
    switch (op)
    {
    case rvOPCODE::LW:
        return "\t" + toString(op) + "\t" + toString(rd) + "," \
                    + std::to_string(imm) + "(" + toString(rs1) +")\n";
    case rvOPCODE::SW:
	// sw	s0,24(sp)
        return "\t" + toString(op) + "\t" + toString(rs2) + "," \
                    + std::to_string(imm) + "(" + toString(rs1) +")\n";
    case rvOPCODE::ADDI:
    case rvOPCODE::XORI:
    case rvOPCODE::ORI:
    case rvOPCODE::ANDI:
    case rvOPCODE::SLTI:
    case rvOPCODE::SLTIU:
	// addi	rd,rs1,imm
        return "\t" + toString(op) + "\t" + \
                      toString(rd) + "," + \
                      toString(rs1) + "," + \
                      std::to_string(imm) + "\n";
    case rvOPCODE::JALR:
    {
        if (label.length()){
            return "\t" + toString(op) + "\t" + \
                        toString(rd) + "," + \
                        toString(rs1) + "," + \
                        label + "\n";
        }
        return "\t" + toString(op) + "\t" + \
                      toString(rd) + "," + \
                      toString(rs1) + "," + \
                      std::to_string(imm) + "\n";
    }
    case rvOPCODE::SLLI:
    case rvOPCODE::SRLI:
    // slli rd, rs1, imm with mask
        return "\t" + toString(op) + "\t" + \
                      toString(rd) + "," + \
                      toString(rs1) + "," + \
                      std::to_string(imm & 0xfffff01f) + "\n";
    case rvOPCODE::SRAI:
    // slli rd, rs1, imm with mask
        return "\t" + toString(op) + "\t" + \
                      toString(rd) + "," + \
                      toString(rs1) + "," + \
                      std::to_string(imm & 0xfffff41f) + "\n";
    case rvOPCODE::BEQ:
    case rvOPCODE::BNE:
    case rvOPCODE::BLT:
    case rvOPCODE::BGE:
    case rvOPCODE::BLTU:
    case rvOPCODE::BGEU:
    // bne  s1,s2,10
    {
        if (label.length()){
            return "\t" + toString(op) + "\t" + \
                        toString(rs1) + "," + \
                        toString(rs2) + "," + \
                        label + "\n";
        }
        return "\t" + toString(op) + "\t" + \
                      toString(rs1) + "," + \
                      toString(rs2) + "," + \
                      std::to_string(imm) + "\n";
    }
    case rvOPCODE::JAL:
    // jal  rd,imm
    {
        if (label.length()){
            return "\t" + toString(op) + "\t" + \
                        toString(rd) + "," + \
                        label + "\n";
        }
        return "\t" + toString(op) + "\t" + \
                      toString(rd) + "," + \
                      std::to_string(imm) + "\n";
    }
    // RV32F / D Floating-Point Extensions
    case rvOPCODE::FLW:
        return "\t" + toString(op) + "\t" + toString(frd) + "," \
                    + std::to_string(imm) + "(" + toString(rs1) +")\n";
    case rvOPCODE::FSW:
        return "\t" + toString(op) + "\t" + toString(frs2) + "," \
                    + std::to_string(imm) + "(" + toString(rs1) +")\n";
    case rvOPCODE::FADD_S:
    case rvOPCODE::FSUB_S:
    case rvOPCODE::FMUL_S:
    case rvOPCODE::FDIV_S:
    case rvOPCODE::FNEQ_S:
    case rvOPCODE::FEQ_S:
    case rvOPCODE::FLT_S:
    case rvOPCODE::FLE_S:
        return "\t" + toString(op) + "\t" + \
                      toString(frd) + "," + \
                      toString(frs1) + "," + \
                      toString(frs2) + "\n";
    case rvOPCODE::FCVT_S_W:        // 整数转浮点数
        return "\t" + toString(op) + "\t" + \
                      toString(frd) + "," + \
                      toString(rs1) + "\n";
    case rvOPCODE::FCVT_W_S:        // 浮点数转整数
        return "\t" + toString(op) + "\t" + \
                      toString(rd) + "," + \
                      toString(frs1) + "\n";
    // 加载符号
    case rvOPCODE::LA:
        return "\t" + toString(op) + "\t" + \
                      toString(rd) + "," + \
                      label + "\n";
    // 加载常量
    case rvOPCODE::LI:
        return "\t" + toString(op) + "\t" + \
                      toString(rd) + "," + \
                      std::to_string(imm) + "\n";
    case rvOPCODE::MOV:
        return "\t" + toString(op) + "\t" + \
                      toString(rd) + "," + \
                      toString(rs1) + "\n";
    case rvOPCODE::FLI:
        assert(0 && "Impossible pseudo instruction!");
    case rvOPCODE::FMOV:
        return "\t" + toString(op) + "\t" + \
                      toString(frd) + "," + \
                      toString(frs1) + "\n";
    case rvOPCODE::J:
    {
        if (label.length()){
            return "\t" + toString(op) + "\t" + label + "\n";
        }
        return "\t" + toString(op) + "\t" + std::to_string(imm) + "\n";
    }
    case rvOPCODE::RET:
        return toString(op) + "\n";
    case rvOPCODE::CALL:
        return "\t" + toString(op) + "\t" + label + "\n";
        // reg move
    case rv::rvOPCODE::FMV_W_X:
        return "\t" + toString(op) + "\t" + \
                      toString(frd) + "," + \
                      toString(rs1) + "\n";
    case rv::rvOPCODE::FMV_X_W: 
        return "\t" + toString(op) + "\t" + \
                      toString(frs1) + "," + \
                      toString(rd) + "\n";
    case rv::rvOPCODE::NOP:
        return "\tnop\t\n";
    default:
    // integer for default, e.g.add rd,rs1,rs2
        return "\t" + toString(op) + "\t" + \
                      toString(rd) + "," + \
                      toString(rs1) + "," + \
                      toString(rs2) + "\n";
    }
}

//rv def


std::string rv::toString(rv::rvREG r){
    switch (r)
    {
    case rv::rvREG::X0: return "zero";
    case rv::rvREG::X1: return "ra";
    case rv::rvREG::X2: return "sp";
    case rv::rvREG::X3: return "gp";
    case rv::rvREG::X4: return "tp";
    case rv::rvREG::X5: return "t0";
    case rv::rvREG::X6: return "t1";
    case rv::rvREG::X7: return "t2";
    case rv::rvREG::X8: return "fp";        // maybe fp
    case rv::rvREG::X9: return "s1";
    case rv::rvREG::X10: return "a0";
    case rv::rvREG::X11: return "a1";
    case rv::rvREG::X12: return "a2";
    case rv::rvREG::X13: return "a3";
    case rv::rvREG::X14: return "a4";
    case rv::rvREG::X15: return "a5";
    case rv::rvREG::X16: return "a6";
    case rv::rvREG::X17: return "a7";
    case rv::rvREG::X18: return "s2";
    case rv::rvREG::X19: return "s3";
    case rv::rvREG::X20: return "s4";
    case rv::rvREG::X21: return "s5";
    case rv::rvREG::X22: return "s6";
    case rv::rvREG::X23: return "s7";
    case rv::rvREG::X24: return "s8";
    case rv::rvREG::X25: return "s9";
    case rv::rvREG::X26: return "s10";
    case rv::rvREG::X27: return "s11";
    case rv::rvREG::X28: return "t3";
    case rv::rvREG::X29: return "t4";
    case rv::rvREG::X30: return "t5";
    case rv::rvREG::X31: return "t6";
    
    default:
        assert(0 && "Unexpected rvREG!");
        break;
    }
}


std::string rv::toString(rv::rvFREG r){
    switch (r)
    {
    case rv::rvFREG::F0: return "ft0";
    case rv::rvFREG::F1: return "ft1";
    case rv::rvFREG::F2: return "ft2";
    case rv::rvFREG::F3: return "ft3";
    case rv::rvFREG::F4: return "ft4";
    case rv::rvFREG::F5: return "ft5";
    case rv::rvFREG::F6: return "ft6";
    case rv::rvFREG::F7: return "ft7";
    case rv::rvFREG::F8: return "fs0";
    case rv::rvFREG::F9: return "fs1";
    case rv::rvFREG::F10: return "fa0";
    case rv::rvFREG::F11: return "fa1";
    case rv::rvFREG::F12: return "fa2";
    case rv::rvFREG::F13: return "fa3";
    case rv::rvFREG::F14: return "fa4";
    case rv::rvFREG::F15: return "fa5";
    case rv::rvFREG::F16: return "fa6";
    case rv::rvFREG::F17: return "fa7";
    case rv::rvFREG::F18: return "fs2";
    case rv::rvFREG::F19: return "fs3";
    case rv::rvFREG::F20: return "fs4";
    case rv::rvFREG::F21: return "fs5";
    case rv::rvFREG::F22: return "fs6";
    case rv::rvFREG::F23: return "fs7";
    case rv::rvFREG::F24: return "fs8";
    case rv::rvFREG::F25: return "fs9";
    case rv::rvFREG::F26: return "fs10";
    case rv::rvFREG::F27: return "fs11";
    case rv::rvFREG::F28: return "fs8";
    case rv::rvFREG::F29: return "ft9";
    case rv::rvFREG::F30: return "ft10";
    case rv::rvFREG::F31: return "ft11";
    
    default:
        assert(0 && "Unexpected rvFREG!");
        break;
    }
}

std::string rv::toString(rv::rvOPCODE r){
    switch (r)
    {
    // arithmetic & logic
    case rv::rvOPCODE::ADD  : return"add";
    case rv::rvOPCODE::SUB  : return"sub";
    case rv::rvOPCODE::XOR  : return"xor";
    case rv::rvOPCODE::OR   : return"or";
    case rv::rvOPCODE::AND  : return"and";
    case rv::rvOPCODE::SLL  : return"sll";
    case rv::rvOPCODE::SRL  : return"srl";
    case rv::rvOPCODE::SRA  : return"sra";
    case rv::rvOPCODE::SLT  : return"slt";
    case rv::rvOPCODE::SLTU : return"sltu";
    // immediate
    case rv::rvOPCODE::ADDI : return"addi";
    case rv::rvOPCODE::XORI : return"xori";
    case rv::rvOPCODE::ORI  : return"ori";
    case rv::rvOPCODE::ANDI : return"andi";
    case rv::rvOPCODE::SLLI : return"slli";
    case rv::rvOPCODE::SRLI : return"srli";
    case rv::rvOPCODE::SRAI : return"srai";
    case rv::rvOPCODE::SLTI : return"slti";
    case rv::rvOPCODE::SLTIU: return"sltiu";
    // load & store
    case rv::rvOPCODE::LW   : return"lw";
    case rv::rvOPCODE::SW   : return"sw";
    // conditional branch
    case rv::rvOPCODE::BEQ  : return"beq";
    case rv::rvOPCODE::BNE  : return"bne";
    case rv::rvOPCODE::BLT  : return"blt";
    case rv::rvOPCODE::BGE  : return"bge";
    case rv::rvOPCODE::BLTU : return"bltu";
    case rv::rvOPCODE::BGEU : return"bgeu";
    // jump
    case rv::rvOPCODE::JAL  : return"jal";
    case rv::rvOPCODE::JALR : return"jalr";
    // RV32M Multiply Extension
    case rv::rvOPCODE::MUL : return"mul";
    case rv::rvOPCODE::DIV : return"div";
    case rv::rvOPCODE::REM : return"rem";
    // RV32F / D Floating-Point Extensions
        // Float Reg Mem Inst  
    case rv::rvOPCODE::FLW  : return"flw";
    case rv::rvOPCODE::FSW  : return"fsw";
        // Operation
    case rv::rvOPCODE::FADD_S  : return"fadd.s";
    case rv::rvOPCODE::FSUB_S  : return"fsub.s";
    case rv::rvOPCODE::FMUL_S  : return"fmul.s";
    case rv::rvOPCODE::FDIV_S  : return"fdiv.s";
        // Convert
    case rv::rvOPCODE::FCVT_S_W  : return"fcvt.s.w";
    case rv::rvOPCODE::FCVT_W_S  : return"fcvt.w.s";
        // Comp Inst 
    case rv::rvOPCODE::FEQ_S  : return"feq.s";
    case rv::rvOPCODE::FLT_S  : return"flt.s";
    case rv::rvOPCODE::FLE_S  : return"fle.s";
        // reg move
    case rv::rvOPCODE::FMV_W_X: return"fmv.w.x";
    case rv::rvOPCODE::FMV_X_W: return"fmv.x.w";
    // Pseudo Instructions
    case rv::rvOPCODE::LA  : return "la";
    case rv::rvOPCODE::LI : return "li";
    case rv::rvOPCODE::MOV  : return "mv";
    case rv::rvOPCODE::FMOV  : return "fmv.s";
    case rv::rvOPCODE::J : return "j";
    case rv::rvOPCODE::RET : return "ret";
    case rv::rvOPCODE::CALL : return "call";
    case rv::rvOPCODE::FNEQ_S  : return"fneq.s";
    case rv::rvOPCODE::NOP  : return"nop";
    default:
        assert(0 && "Unexpected Operation!");
        break;
    }
}