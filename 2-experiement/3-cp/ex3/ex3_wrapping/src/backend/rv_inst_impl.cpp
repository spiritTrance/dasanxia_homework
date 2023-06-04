#include "backend/rv_inst_impl.h"
#include <assert.h>

// impl
std::string rv::rv_inst::draw(std::string comment) const{
    switch (op)
    {
    case rvOPCODE::LW:
        return "\t" + toString(op) + "\t" + toString(rd) + "," \
                    + std::to_string(imm) + "(" + toString(rs1) +")" + comment + "\n";
    case rvOPCODE::SW:
	// sw	s0,24(sp)
        return "\t" + toString(op) + "\t" + toString(rs2) + "," \
                    + std::to_string(imm) + "(" + toString(rs1) +")" + comment + "\n";
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
                      std::to_string(imm)  + comment +  "\n";
    case rvOPCODE::JALR:
    {
        if (label.length()){
            return "\t" + toString(op) + "\t" + \
                        toString(rd) + "," + \
                        toString(rs1) + "," + \
                        label  + comment +  "\n";
        }
        return "\t" + toString(op) + "\t" + \
                      toString(rd) + "," + \
                      toString(rs1) + "," + \
                      std::to_string(imm)  + comment +  "\n";
    }
    case rvOPCODE::SLLI:
    case rvOPCODE::SRLI:
    // slli rd, rs1, imm with mask
        return "\t" + toString(op) + "\t" + \
                      toString(rd) + "," + \
                      toString(rs1) + "," + \
                      std::to_string(imm & 0xfffff01f)  + comment +  "\n";
    case rvOPCODE::SRAI:
    // slli rd, rs1, imm with mask
        return "\t" + toString(op) + "\t" + \
                      toString(rd) + "," + \
                      toString(rs1) + "," + \
                      std::to_string(imm & 0xfffff41f)  + comment +  "\n";
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
                        label  + comment +  "\n";
        }
        return "\t" + toString(op) + "\t" + \
                      toString(rs1) + "," + \
                      toString(rs2) + "," + \
                      std::to_string(imm)  + comment +  "\n";
    }
    case rvOPCODE::JAL:
    // jal  rd,imm
    {
        if (label.length()){
            return "\t" + toString(op) + "\t" + \
                        toString(rd) + "," + \
                        label  + comment +  "\n";
        }
        return "\t" + toString(op) + "\t" + \
                      toString(rd) + "," + \
                      std::to_string(imm)  + comment +  "\n";
    }
    // RV32F / D Floating-Point Extensions
    case rvOPCODE::FLW:
        return "\t" + toString(op) + "\t" + toString(frd) + "," \
                    + std::to_string(imm) + "(" + toString(rs1) +")" + comment + "\n";
    case rvOPCODE::FSW:
        return "\t" + toString(op) + "\t" + toString(frs2) + "," \
                    + std::to_string(imm) + "(" + toString(rs1) +")" + comment + "\n";
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
                      toString(frs2) + comment +  "\n";
    case rvOPCODE::FCVT_S_W:        // 整数转浮点数
        return "\t" + toString(op) + "\t" + \
                      toString(frd) + "," + \
                      toString(rs1) + comment + "\n";
    case rvOPCODE::FCVT_W_S:        // 浮点数转整数
        return "\t" + toString(op) + "\t" + \
                      toString(rd) + "," + \
                      toString(frs1) + comment + "\n";
    // 加载符号
    case rvOPCODE::LA:
        return "\t" + toString(op) + "\t" + \
                      toString(rd) + "," + \
                      label + comment + "\n";
    // 加载常量
    case rvOPCODE::LI:
        return "\t" + toString(op) + "\t" + \
                      toString(rd) + "," + \
                      std::to_string(imm) + comment + "\n";
    case rvOPCODE::MOV:
        return "\t" + toString(op) + "\t" + \
                      toString(rd) + "," + \
                      toString(rs1) + comment + "\n";
    case rvOPCODE::FLI:
        assert(0 && "Impossible pseudo instruction!");
    case rvOPCODE::FMOV:
        return "\t" + toString(op) + "\t" + \
                      toString(frd) + "," + \
                      toString(frs1) + comment + "\n";
    case rvOPCODE::J:
    {
        if (label.length()){
            return "\t" + toString(op) + "\t" + label + comment + "\n";
        }
        return "\t" + toString(op) + "\t" + std::to_string(imm) + comment + "\n";
    }
    case rvOPCODE::RET:
        return toString(op) + comment + "\n";
    case rvOPCODE::CALL:
        return "\t" + toString(op) + "\t" + label + comment + "\n";
        // reg move
    case rv::rvOPCODE::FMV_W_X:
        return "\t" + toString(op) + "\t" + \
                      toString(frd) + "," + \
                      toString(rs1) + comment + "\n";
    case rv::rvOPCODE::FMV_X_W: 
        return "\t" + toString(op) + "\t" + \
                      toString(frs1) + "," + \
                      toString(rd) + comment + "\n";
    case rv::rvOPCODE::NOP:
        return "\tnop\t" + comment + "\n";
    default:
    // integer for default, e.g.add rd,rs1,rs2
        return "\t" + toString(op) + "\t" + \
                      toString(rd) + "," + \
                      toString(rs1) + "," + \
                      toString(rs2) + comment + "\n";
    }
}
