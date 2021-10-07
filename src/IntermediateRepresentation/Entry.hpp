#ifndef IR_ENTRY
#define IR_ENTRY

#include "Operand.hpp"

namespace icode
{
    enum Instruction
    {
        PASS,
        PASS_ADDR,
        PASS_PTR,
        CALL,
        RET,
        ADDR_ADD,
        ADDR_MUL,
        EQUAL,
        PTR_ASSIGN,
        READ,
        WRITE,
        CREATE_PTR,
        ALLOC_PTR,
        CAST,
        PTR_CAST,
        UNARY_MINUS,
        NOT,
        MUL,
        DIV,
        MOD,
        ADD,
        SUB,
        RSH,
        LSH,
        LT,
        LTE,
        GT,
        GTE,
        EQ,
        NEQ,
        BWA,
        BWX,
        BWO,
        GOTO,
        IF_TRUE_GOTO,
        IF_FALSE_GOTO,
        CREATE_LABEL,
        PRINT,
        PRINT_STR,
        SPACE,
        NEWLN,
        INPUT,
        INPUT_STR,
        MEMCPY
    };

    struct Entry
    {
        Instruction opcode;
        Operand op1;
        Operand op2;
        Operand op3;
    };
}

#endif