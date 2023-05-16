#include"backend/rv_def.h"
#include<assert.h>

#define TODO assert(0 && "TODO");

std::string rv::toDebugString(rv::rvREG r){
    TODO;
}

std::string rv::toGenerateString(rv::rvREG r){
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
    case rv::rvREG::X8: return "s0";        // maybe fp
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

std::string rv::toDebugString(rv::rvFREG r){
    TODO;
}

std::string rv::toGenerateString(rv::rvFREG r){
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
    // Pseudo Instructions
    case rv::rvOPCODE::LA  : return "la";
    case rv::rvOPCODE::LI : return "li";
    case rv::rvOPCODE::MOV  : return "mv";
    case rv::rvOPCODE::J : return "j";
    case rv::rvOPCODE::RET : return "ret";
    case rv::rvOPCODE::CALL : return "call";
    default:
        assert(0 && "Unexpected Operation!");
        break;
    }
}