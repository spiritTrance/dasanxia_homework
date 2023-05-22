#include "ir/ir_operand.h"

#include <string>
#include <utility>


ir::Operand::Operand(std::string  n, Type t): name(std::move(n)), type(t) {}


bool ir::Operand::operator<(const ir::Operand & opd) const{
    if (this->name != opd.name){
        return this->type < opd.type;
    }
    return this->name < opd.name;
}
bool ir::Operand::operator>(const ir::Operand & opd) const{
    if (this->name != opd.name){
        return this->type > opd.type;
    }
    return this->name > opd.name;
}
