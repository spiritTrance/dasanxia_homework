#include "ir/ir_operand.h"

#include <string>
#include <utility>

// 静态库不重载一下。。。6
bool ir::Operand::operator<(const ir::Operand & opd) const{
    if (this->name != opd.name){
        return this->name < opd.name;
    }
    return this->type < opd.type;
}
bool ir::Operand::operator>(const ir::Operand & opd) const{
    if (this->name != opd.name){
        return this->name > opd.name;
    }
    return this->type > opd.type;
}
