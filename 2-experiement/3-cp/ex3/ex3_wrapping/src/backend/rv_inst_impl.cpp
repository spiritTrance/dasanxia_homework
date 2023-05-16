#include"backend/rv_inst_impl.h"
#include<assert.h>

#define TODO assert(0 && "todo")

// 注意有的imm是不是可以用label代替！！！
// 注意有的imm是不是可以用label代替！！！
// 注意有的imm是不是可以用label代替！！！
// 凡是涉及跳转的就加上，否则不加
// fnmdp，%hi和%lo被你吃了是吧，加！
std::string rv::rv_inst::draw() const{
    switch (op)
    {
    case rvOPCODE::LW:
        return "\t" + toString(op) + "\t" + toGenerateString(rd) + "," \
                    + std::to_string(imm) + "(" + toGenerateString(rs1) +")";
    case rvOPCODE::SW:
	// sw	s0,24(sp)
        return "\t" + toString(op) + "\t" + toGenerateString(rs2) + "," \
                    + std::to_string(imm) + "(" + toGenerateString(rs1) +")";
    case rvOPCODE::ADDI:
    case rvOPCODE::XORI:
    case rvOPCODE::ORI:
    case rvOPCODE::ANDI:
    case rvOPCODE::SLTI:
    case rvOPCODE::SLTIU:
	// addi	rd,rs1,imm
        return "\t" + toString(op) + "\t" + \
                      toGenerateString(rd) + "," + \
                      toGenerateString(rs1) + "," + \
                      std::to_string(imm);
    case rvOPCODE::JALR:
    {
        if (label.length()){
            return "\t" + toString(op) + "\t" + \
                        toGenerateString(rd) + "," + \
                        toGenerateString(rs1) + "," + \
                        label;
        }
        return "\t" + toString(op) + "\t" + \
                      toGenerateString(rd) + "," + \
                      toGenerateString(rs1) + "," + \
                      std::to_string(imm);
    }
    case rvOPCODE::SLLI:
    case rvOPCODE::SRLI:
    // slli rd, rs1, imm with mask
        return "\t" + toString(op) + "\t" + \
                      toGenerateString(rd) + "," + \
                      toGenerateString(rs1) + "," + \
                      std::to_string(imm & 0xfffff01f);
    case rvOPCODE::SRAI:
    // slli rd, rs1, imm with mask
        return "\t" + toString(op) + "\t" + \
                      toGenerateString(rd) + "," + \
                      toGenerateString(rs1) + "," + \
                      std::to_string(imm & 0xfffff41f);
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
                        toGenerateString(rs1) + "," + \
                        toGenerateString(rs2) + "," + \
                        label;
        }
        return "\t" + toString(op) + "\t" + \
                      toGenerateString(rs1) + "," + \
                      toGenerateString(rs2) + "," + \
                      std::to_string(imm);
    }
    case rvOPCODE::JAL:
    // jal  rd,imm
    {
        if (label.length()){
            return "\t" + toString(op) + "\t" + \
                        toGenerateString(rd) + "," + \
                        label;
        }
        return "\t" + toString(op) + "\t" + \
                      toGenerateString(rd) + "," + \
                      std::to_string(imm);
    }
    // RV32F / D Floating-Point Extensions
    case rvOPCODE::FLW:
        return "\t" + toString(op) + "\t" + toGenerateString(frd) + "," \
                    + std::to_string(imm) + "(" + toGenerateString(rs1) +")";
    case rvOPCODE::FSW:
        return "\t" + toString(op) + "\t" + toGenerateString(frs2) + "," \
                    + std::to_string(imm) + "(" + toGenerateString(rs1) +")";
    case rvOPCODE::FADD_S:
    case rvOPCODE::FSUB_S:
    case rvOPCODE::FMUL_S:
    case rvOPCODE::FDIV_S:
    case rvOPCODE::FEQ_S:
    case rvOPCODE::FLT_S:
    case rvOPCODE::FLE_S:
        return "\t" + toString(op) + "\t" + \
                      toGenerateString(frd) + "," + \
                      toGenerateString(frs1) + "," + \
                      toGenerateString(frs2);
    case rvOPCODE::FCVT_S_W:        // 整数转浮点数
        return "\t" + toString(op) + "\t" + \
                      toGenerateString(frd) + "," + \
                      toGenerateString(rs1);
    case rvOPCODE::FCVT_W_S:        // 浮点数转整数
        return "\t" + toString(op) + "\t" + \
                      toGenerateString(rd) + "," + \
                      toGenerateString(frs1);
    // 加载符号
    case rvOPCODE::LA:
        return "\t" + toString(op) + "\t" + \
                      toGenerateString(rd) + "," + \
                      label;
    // 加载常量
    case rvOPCODE::LI:
        return "\t" + toString(op) + "\t" + \
                      toGenerateString(rd) + "," + \
                      std::to_string(imm);
    case rvOPCODE::MOV:
        return "\t" + toString(op) + "\t" + \
                      toGenerateString(rd) + "," + \
                      toGenerateString(rs1);
    case rvOPCODE::FLI:
        return "\t" + toString(op) + "\t" + \
                      toGenerateString(frd) + "," + \
                      std::to_string(imm);
    case rvOPCODE::FMOV:
        return "\t" + toString(op) + "\t" + \
                      toGenerateString(frd) + "," + \
                      toGenerateString(frs1);
    case rvOPCODE::J:
    {
        if (label.length()){
            return "\t" + toString(op) + "\t" + label;
        }
        return "\t" + toString(op) + "\t" + std::to_string(imm);
    }
    case rvOPCODE::RET:
        return toString(op);
    case rvOPCODE::CALL:
        return "\t" + toString(op) + "\t" + label;
    default:
    // integer for default, e.g.add rd,rs1,rs2
        return "\t" + toString(op) + "\t" + \
                      toGenerateString(rd) + "," + \
                      toGenerateString(rs1) + "," + \
                      toGenerateString(rs2);
    }
}