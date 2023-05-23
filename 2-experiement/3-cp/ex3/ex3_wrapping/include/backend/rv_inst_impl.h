#ifndef RV_INST_IMPL_H
#define RV_INST_IMPL_H

#include "backend/rv_def.h"

namespace rv {

struct rv_inst {
    rvREG rd, rs1, rs2;         // operands of rv integer inst
    rvFREG frd, frs1, frs2;         // operands of rv float inst
    rvOPCODE op;                // opcode of rv inst
    
    int32_t imm;               // optional, in immediate inst
    std::string label;          // optional, in beq/jarl inst

    std::string draw() const;
};

};

#endif