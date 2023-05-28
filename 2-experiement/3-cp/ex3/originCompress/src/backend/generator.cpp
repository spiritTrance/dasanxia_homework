#include"backend/generator.h"

#include<assert.h>
#include<set>
#include<map>

#define TODO assert(0 && "todo")

backend::Generator::Generator(ir::Program& p, std::ofstream& f): program(p), fout(f) {}

void backend::Generator::gen() {
    // case 1: s.insert() 引发 JSON格式错误
    // std::set<ir::Operand> s;
    // s.insert(ir::Operand());
    // case 2: instance._table的下标访问和find 均能引发 JSON格式错误
    stackVarMap instance;
    // instance._table[ir::Operand()] = -3;
    // auto it = instance._table.find(ir::Operand());
    return;
}