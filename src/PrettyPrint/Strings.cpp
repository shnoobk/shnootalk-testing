#include "Strings.hpp"

const std::string tokenTypeToString[] = { "NONE",
                                          "SPACE",
                                          "FUNCTION",
                                          "EXTERN_FUNCTION",
                                          "IDENTIFIER",
                                          "LPAREN",
                                          "RPAREN",
                                          "STRUCT",
                                          "ENUM",
                                          "DEF",
                                          "BEGIN",
                                          "END",
                                          "COLON",
                                          "DOUBLE_COLON",
                                          "COMMA",
                                          "DOT",
                                          "VAR",
                                          "CONST",
                                          "STR_LITERAL",
                                          "CHAR_LITERAL",
                                          "INT_LITERAL",
                                          "HEX_LITERAL",
                                          "BIN_LITERAL",
                                          "FLOAT_LITERAL",
                                          "IF",
                                          "ELSEIF",
                                          "ELSE",
                                          "WHILE",
                                          "DO",
                                          "FOR",
                                          "LOOP",
                                          "BREAK",
                                          "CONTINUE",
                                          "RETURN",
                                          "VOID",
                                          "EQUAL",
                                          "PLUS_EQUAL",
                                          "MINUS_EQUAL",
                                          "DIVIDE_EQUAL",
                                          "MULTIPLY_EQUAL",
                                          "OR_EQUAL",
                                          "AND_EQUAL",
                                          "XOR_EQUAL",
                                          "NOT",
                                          "CONDN_NOT",
                                          "CAST",
                                          "OPEN_SQUARE",
                                          "CLOSE_SQUARE",
                                          "OPEN_BRACE",
                                          "CLOSE_BRACE",
                                          "RIGHT_ARROW",
                                          "PLUS",
                                          "MINUS",
                                          "BITWISE_OR",
                                          "BITWISE_XOR",
                                          "RIGHT_SHIFT",
                                          "LEFT_SHIFT",
                                          "CONDN_OR",
                                          "GREATER_THAN",
                                          "LESS_THAN",
                                          "GREATER_THAN_EQUAL",
                                          "LESS_THAN_EQUAL",
                                          "CONDN_EQUAL",
                                          "CONDN_NOT_EQUAL",
                                          "MULTIPLY",
                                          "DIVIDE",
                                          "MOD",
                                          "BITWISE_AND",
                                          "CONDN_AND",
                                          "USE",
                                          "AS",
                                          "FROM",
                                          "MUTABLE",
                                          "SEMICOLON",
                                          "END_OF_LINE",
                                          "END_OF_FILE",
                                          "INVALID",
                                          "SIZEOF",
                                          "TYPEOF",
                                          "PRINT",
                                          "PRINTLN",
                                          "INPUT" };

const std::string nodeTypeToString[] = { "PROGRAM",
                                         "USE",
                                         "FROM",
                                         "BLOCK",
                                         "MODULE",
                                         "FUNCTION",
                                         "EXTERN_FUNCTION",
                                         "STRUCT",
                                         "ENUM",
                                         "DEF",
                                         "VAR",
                                         "CONST",
                                         "IF",
                                         "ELSEIF",
                                         "ELSE",
                                         "WHILE",
                                         "DO_WHILE",
                                         "FOR",
                                         "LOOP",
                                         "CAST",
                                         "BREAK",
                                         "CONTINUE",
                                         "ASSIGNMENT",
                                         "RETURN",
                                         "FUNCCALL",
                                         "METHODCALL",
                                         "PARAM",
                                         "MUT_PARAM",
                                         "EXPRESSION",
                                         "STR_LITERAL",
                                         "LITERAL",
                                         "IDENTIFIER",
                                         "ASSIGN_OPERATOR",
                                         "OPERATOR",
                                         "UNARY_OPR",
                                         "SUBSCRIPT",
                                         "STRUCT_VAR",
                                         "INITLIST",
                                         "TERM",
                                         "SIZEOF",
                                         "TYPEOF",
                                         "PRINT",
                                         "PRINTLN",
                                         "SPACE",
                                         "INPUT" };

const std::string operandTypeToString[] = { "temp",      "tempPtr", "bytes",       "stringData",        "var",
                                            "globalVar", "ptr",     "returnValue", "calleeReturnValue", "literal",
                                            "label",     "module",  "none" };

const std::string instructionToString[] = { "pass",        "passAddress",
                                            "call",        "return",
                                            "addressAdd",  "addressMultiply",
                                            "equal",       "read",
                                            "write",       "createPointer",
                                            "cast",        "unaryMinus",
                                            "not",         "multiply",
                                            "divide",      "remainder",
                                            "add",         "subtract",
                                            "rightShift",  "leftShift",
                                            "lessThan",    "lessThanOrEqualTo",
                                            "greaterThan", "greaterThanOrEqualTo",
                                            "isEqual",     "isNotEqual",
                                            "bitwiseAnd",  "bitwiseXor",
                                            "bitwiseOr",   "goto",
                                            "gotoIfTrue",  "gotoIfFalse",
                                            "createLabel", "print",
                                            "printString", "space",
                                            "newLine",     "input",
                                            "inputString", "memoryCopy" };