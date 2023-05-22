#ifndef IROPERAND_H
#define IROPERAND_H

#include <string>


namespace ir {

enum class Type {
    Int,
    Float,
    IntLiteral,
    FloatLiteral,
    IntPtr,
    FloatPtr,
    null
};

std::string toString(Type t);

struct Operand {
    std::string name;
    Type type;
    Operand(std::string = "null", Type = Type::null);
    bool operator<(const ir::Operand &) const;
    bool operator>(const ir::Operand &) const;
    // 修改bug：不要重载等于号
};

}
#endif
