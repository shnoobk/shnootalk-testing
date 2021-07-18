#ifndef PP_IR_PRINTER
#define PP_IR_PRINTER

#include <fstream>
#include <iostream>
#include <string>

#include "../IntermediateRepresentation/All.hpp"
#include "../Node/Node.hpp"
#include "../Token/Token.hpp"
#include "Strings.hpp"

namespace pp
{
    void printToken(const Token& symbol);

    void printNode(const Node& node, int depth = 1);

    void printOperand(const icode::Operand& op);

    void printEntry(const icode::Entry& entry);

    void printTypeDescription(const icode::TypeDescription& var_info);

    void printStructDescription(const icode::StructDescription& struct_desc, int ilvl = 0);

    void printFunctionDescription(const icode::FunctionDescription& func_desc, int ilvl = 0);

    void printModuleDescription(const icode::ModuleDescription& module_desc, int ilvl = 0);
}

#endif