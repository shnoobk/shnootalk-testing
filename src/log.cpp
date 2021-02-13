#include "log.hpp"

namespace log
{
    static std::string token_type_strs[] = { "NONE",
                                             "SPACE",
                                             "FUNCTION",
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
                                             "FOR",
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
                                             "OPEN_SQAURE",
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
                                             "INPUT",
                                             "EXIT" };

    static std::string node_type_strs[] = { "PROGRAM",
                                            "USE",
                                            "FROM",
                                            "BLOCK",
                                            "MODULE",
                                            "FUNCTION",
                                            "STRUCT",
                                            "ENUM",
                                            "DEF",
                                            "VAR",
                                            "CONST",
                                            "IF",
                                            "ELSEIF",
                                            "ELSE",
                                            "WHILE",
                                            "FOR",
                                            "RET_TYPE",
                                            "CAST",
                                            "BREAK",
                                            "CONTINUE",
                                            "ASSIGNMENT",
                                            "ASSIGNMENT_STR",
                                            "ASSIGNMENT_INITLIST",
                                            "RETURN",
                                            "FUNCCALL",
                                            "STRUCT_FUNCCALL",
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
                                            "INPUT",
                                            "EXIT" };

    static std::string data_type_strs[] = { "I8",  "I16",    "I32",     "UI32",
                                            "F32", "VM_INT", "VM_UINT", "VM_FLOAT",
                                            "INT", "FLOAT",  "STRUCT",  "VOID" };

    static std::string instruction_strs[] = {
        "PASS",          "PASS_ADDR",    "CALL",      "RET",       "ADDR_ADD",
        "ADDR_MUL",      "EQUAL",        "READ",      "WRITE",     "CAST",
        "UNARY_MINUS",   "NOT",          "MUL",       "DIV",       "MOD",
        "ADD",           "SUB",          "RSH",       "LSH",       "LT",
        "LTE",           "GT",           "GTE",       "EQ",        "NEQ",
        "BWA",           "BWX",          "BWO",       "GOTO",      "IF_TRUE_GOTO",
        "IF_FALSE_GOTO", "CREATE_LABEL", "PRINT",     "PRINT_STR", "SPACE",
        "NEWLN",         "INPUT",        "INPUT_STR", "EXIT"
    };

    static std::string vm_instruction_strs[] = {
        "GBLMODE", "FPMODE",   "LDFP",     "ADDFP",    "SUBFP",   "LDEA",    "ADDEA",
        "LD",      "LDPTR",    "LDIMf",    "LDIMi",    "LDIMui",  "ST",      "STPTR",
        "CASTfi",  "CASTfui",  "CASTif",   "CASTiui",  "CASTuii", "CASTuif", "NOT",
        "BWA",     "BWX",      "BWO",      "RSH",      "RSHi",    "LSH",     "BWAIM",
        "BWXIM",   "BWOIM",    "RSHIM",    "RSHIMi",   "LSHIM",   "NEGf",    "NEGi",
        "NEGui",   "MULi",     "DIVi",     "MODi",     "ADDi",    "SUBi",    "LTi",
        "GTi",     "EQi",      "MULf",     "DIVf",     "MODf",    "ADDf",    "SUBf",
        "LTf",     "GTf",      "EQf",      "MULui",    "DIVui",   "MODui",   "ADDui",
        "SUBui",   "LTui",     "GTui",     "EQui",     "MULIMi",  "DIVIMi",  "MODIMi",
        "ADDIMi",  "SUBIMi",   "LTIMi",    "GTIMi",    "EQIMi",   "MULIMf",  "DIVIMf",
        "MODIMf",  "ADDIMf",   "SUBIMf",   "LTIMf",    "GTIMf",   "EQIMf",   "MULIMui",
        "DIVIMui", "MODIMui",  "ADDIMui",  "SUBIMui",  "LTIMui",  "GTIMui",  "EQIMui",
        "JMP",     "CALL",     "JMPF",     "JMPNF",    "RET",     "PRINTf",  "PRINTi",
        "PRINTui", "PRINTstr", "PRINTdat", "PRINTPtr", "NEWLN",   "SPACE",   "INPUTf",
        "INPUTi",  "INPUTui",  "INPUTstr", "INPUTptr", "INPLIM",  "EXIT",
    };

    void print_token(const token::token& symbol)
    {
        /* Prints token and its properties */

        std::cout << "Token(\"" << symbol.str << "\", " << token_type_strs[symbol.type]
                  << ", line=" << symbol.lineno << ", col=" << symbol.col << ")";
    }

    void print_node(const node::node& node, int depth)
    {
        /* Recursively prints tree, used to print AST */

        static std::vector<bool> last_child;

        std::cout << "Node(" << node_type_strs[node.type] << ", ";
        print_token(node.tok);
        std::cout << ")" << std::endl;

        last_child.push_back(false);

        for (size_t i = 0; i < node.children.size(); i++)
        {
            for (size_t j = 1; j <= depth; j++)
                if (j == depth)
                    if (i == node.children.size() - 1)
                        std::cout << "└"
                                  << "──";
                    else
                        std::cout << "├"
                                  << "──";
                else if (last_child[j - 1])
                    std::cout << " "
                              << "  ";
                else
                    std::cout << "│"
                              << "  ";

            if (i == node.children.size() - 1)
                last_child[depth - 1] = true;

            print_node(node.children[i], depth + 1);
        }

        last_child.pop_back();
    }

    void println(const std::string& msg)
    {
        /* Prints string */

        std::cout << msg << std::endl;
    }

    void
    error_line(const std::string& error_msg, const std::string& line, int lineno, int col)
    {
        /* Accepts line as string and column, prints line and '^' symbol at col
            along with error message */

        std::cout << "ERROR in ";
        std::cout << "Line " << lineno;
        std::cout << " Col " << col + 1 << std::endl;
        std::cout << std::endl;
        std::cout << line << std::endl;
        std::cout << std::string(col, ' ') << "^" << std::endl;
        std::cout << error_msg << std::endl;
    }

    void error(const std::string& mod_name, const std::string& error_msg)
    {
        println("MODULE " + mod_name);
        println(error_msg);
    }

    void error_tok(const std::string& mod_name,
                   const std::string& error_msg,
                   std::ifstream& file,
                   const token::token& tok)
    {
        /* Prints the exact line from file using tok.lineno,
            and error message */

        /* Goto beginning */
        file.clear();
        file.seekg(0, std::ios::beg);

        /* Get line */
        std::string line;
        for (size_t i = 0; i < tok.lineno; i++)
            getline(file, line);

        println("MODULE " + mod_name);
        error_line(error_msg, line, tok.lineno, tok.col);
    }

    void parse_error(const std::string& mod_name,
                     token::token_type expected,
                     token::token& found,
                     std::ifstream& file)
    {
        /* Used by parser when it finds some other token type than expected */

        std::string error_msg = "Did not expect " + token_type_strs[found.type];
        error_msg += ",\nexpected " + token_type_strs[expected];
        error_tok(mod_name, error_msg, file, found);
    }

    void parse_error_mult(const std::string& mod_name,
                          const token::token_type* expected,
                          int ntoks,
                          const token::token& found,
                          std::ifstream& file)
    {
        /* Used by parser when if finds a token type that does not match
           multiple token types expected */

        std::string error_msg = "Did not expect " + token_type_strs[found.type];
        error_msg += ",\nexpected ";

        for (size_t i = 0; i < ntoks - 1; i++)
            error_msg += ((token_type_strs[expected[i]]) + " or ");

        error_msg += token_type_strs[expected[ntoks - 1]];

        error_tok(mod_name, error_msg, file, found);
    }

    std::string str_var_info(icode::var_info& var)
    {
        std::string var_str = var.dtype_name;

        for (unsigned int dim : var.dimensions)
            var_str += "[" + std::to_string(dim) + "]";

        return var_str;
    }

    void type_error(const std::string& mod_name,
                    std::ifstream& file,
                    const token::token& tok,
                    icode::var_info& expected,
                    icode::var_info& found)
    {
        /* Used by ir generator for type errors */

        std::string found_str = str_var_info(found);
        std::string expected_str = str_var_info(expected);

        std::string expect_msg = "Type error, did not expect " + found_str;
        expect_msg += ",\nexpected " + expected_str;

        error_tok(mod_name, expect_msg, file, tok);
    }

    void internal_error_tok(const std::string& mod_name,
                            std::ifstream& file,
                            const token::token& tok)
    {
        error_tok(mod_name, "Internal compiler error, REPORT THIS BUG", file, tok);
    }

    void internal_error(const std::string& mod_name)
    {
        error(mod_name, "Internal compiler error, REPORT THIS BUG");
    }

    void print_operand(const icode::operand& op)
    {
        switch (op.optype)
        {
            case icode::TEMP:
                std::cout << "Temp(id_" << op.temp_id << ":" << data_type_strs[op.dtype];
                break;
            case icode::TEMP_PTR:
                std::cout << "TempPtr(id_" << op.temp_id << ":"
                          << data_type_strs[op.dtype];
                break;
            case icode::STR_DATA:
                std::cout << "StrDat(name=" << op.name << " size=" << op.val.size;
                break;
            case icode::ADDR:
                std::cout << "Addr(" << op.val.address;
                break;
            case icode::GBL_ADDR:
                std::cout << "GblAddr(" << op.val.address;
                break;
            case icode::FP_ADDR:
                std::cout << "FpAddr(" << op.val.address;
                break;
            case icode::VAR:
                std::cout << "Var(" << op.name << ":" << data_type_strs[op.dtype];
                break;
            case icode::GBL_VAR:
                std::cout << "GblVar(" << op.name << ":" << data_type_strs[op.dtype];
                break;
            case icode::PTR:
                std::cout << "Ptr(" << op.name << ":" << data_type_strs[op.dtype];
                break;
            case icode::RET_PTR:
                std::cout << "RetPtr("
                          << "id_" << op.temp_id;
                break;
            case icode::LITERAL:
            {
                if (icode::is_uint(op.dtype))
                {
                    std::cout << "Ltrl(" << op.val.size << ":"
                              << data_type_strs[op.dtype];
                }
                else if (icode::is_int(op.dtype))
                {
                    std::cout << "Ltrl(" << op.val.integer << ":"
                              << data_type_strs[op.dtype];
                }
                else
                {
                    std::cout << "Ltrl(" << op.val.floating << ":"
                              << data_type_strs[op.dtype];
                }

                break;
            }
            case icode::DTYPE:
                std::cout << "Dtype(" << data_type_strs[op.dtype];
                break;
            case icode::LABEL:
                std::cout << "Label(" << op.name;
                break;
            case icode::MODULE:
                std::cout << "Module(" << op.name;
                break;
            case icode::NONE:
                return;
                break;
        }

        std::cout << ")";
    }

    void print_entry(const icode::entry& entry)
    {
        std::cout << instruction_strs[entry.opcode] << " ";
        print_operand(entry.op1);
        std::cout << " ";
        print_operand(entry.op2);
        std::cout << " ";
        print_operand(entry.op3);
    }

    void print_var_info(const icode::var_info& var_info)
    {
        std::cout << "Var(";
        std::cout << "dtype="
                  << "\"" << var_info.dtype_name << "\":";
        std::cout << data_type_strs[var_info.dtype];
        std::cout << " dtypesize=" << var_info.dtype_size;
        std::cout << " mod="
                  << "\"" << var_info.module_name << "\"";
        std::cout << " offset=" << var_info.offset;
        std::cout << " size=" << var_info.size;

        std::cout << " dim=[";
        for (unsigned int i : var_info.dimensions)
            std::cout << i << ",";
        std::cout << "]";

        std::cout << ")";
    }

    void print_struct_desc(const icode::struct_desc& struct_desc, int ilvl)
    {
        std::cout << "Struct(" << std::endl;

        std::cout << std::string(ilvl + 3, ' ') << "Fields={" << std::endl;
        for (auto field : struct_desc.fields)
        {
            std::cout << std::string(ilvl + 6, ' ');
            std::cout << field.first << ":";
            print_var_info(field.second);
            std::cout << std::endl;
        }
        std::cout << std::string(ilvl + 3, ' ') << "}" << std::endl;

        std::cout << std::string(ilvl + 3, ' ') << "Size=" << struct_desc.size
                  << std::endl;

        std::cout << std::string(ilvl + 3, ' ') << "Module=" << struct_desc.size
                  << std::endl;

        std::cout << std::string(ilvl, ' ') << ")" << std::endl;
    }

    void print_def(const icode::def& definition)
    {
        std::cout << "Def(";

        if (definition.dtype == icode::INT)
            std::cout << definition.val.integer;
        else
            std::cout << definition.val.floating;

        std::cout << ":" << data_type_strs[definition.dtype] << ")";
    }

    void print_func_desc(const icode::func_desc& func_desc, int ilvl)
    {
        std::cout << "Func(" << std::endl;

        std::cout << std::string(ilvl + 3, ' ') << "Params=[";
        for (std::string param : func_desc.params)
            std::cout << param << ", ";
        std::cout << "]" << std::endl;

        std::cout << std::string(ilvl + 3, ' ') << "Retinfo=";
        print_var_info(func_desc.func_info);
        std::cout << std::endl;

        std::cout << std::string(ilvl + 3, ' ') << "Symbols={" << std::endl;
        for (auto symbol : func_desc.symbols)
        {
            std::cout << std::string(ilvl + 6, ' ');
            std::cout << symbol.first << ":";
            print_var_info(symbol.second);
            std::cout << std::endl;
        }
        std::cout << std::string(ilvl + 3, ' ') << "}" << std::endl;

        std::cout << std::string(ilvl + 3, ' ') << "icode=[" << std::endl;
        for (icode::entry i : func_desc.icode_table)
        {
            std::cout << std::string(ilvl + 6, ' ');
            print_entry(i);
            std::cout << std::endl;
        }
        std::cout << std::string(ilvl + 3, ' ') << "]" << std::endl;

        std::cout << std::string(ilvl, ' ') << ")" << std::endl;
    }

    void print_module_desc(const icode::module_desc& module_desc, int ilvl)
    {
        std::cout << "Module(" << std::endl;

        std::cout << std::string(ilvl + 3, ' ') << "Name=";
        std::cout << "\"" + module_desc.name + "\"" << std::endl;

        std::cout << std::string(ilvl + 3, ' ') << "Uses=[";
        for (std::string use : module_desc.uses)
            std::cout << use << ", ";
        std::cout << "]" << std::endl;

        std::cout << std::string(ilvl + 3, ' ') << "Enums={" << std::endl;
        for (auto enumeration : module_desc.enumerations)
        {
            std::cout << std::string(ilvl + 6, ' ');
            std::cout << enumeration.first << "=";
            std::cout << enumeration.second << std::endl;
        }
        std::cout << std::string(ilvl + 3, ' ') << "}" << std::endl;

        std::cout << std::string(ilvl + 3, ' ') << "Defs={" << std::endl;
        for (auto definition : module_desc.defines)
        {
            std::cout << std::string(ilvl + 6, ' ');
            std::cout << definition.first << ":";
            print_def(definition.second);
            std::cout << std::endl;
        }
        std::cout << std::string(ilvl + 3, ' ') << "}" << std::endl;

        std::cout << std::string(ilvl + 3, ' ') << "Globals={" << std::endl;
        for (auto symbol : module_desc.globals)
        {
            std::cout << std::string(ilvl + 6, ' ');
            std::cout << symbol.first << ":";
            print_var_info(symbol.second);
            std::cout << std::endl;
        }
        std::cout << std::string(ilvl + 3, ' ') << "}" << std::endl;

        std::cout << std::string(ilvl + 3, ' ') << "Sructs={" << std::endl;
        for (auto structure : module_desc.structures)
        {
            std::cout << std::string(ilvl + 6, ' ');
            std::cout << structure.first << ":";
            print_struct_desc(structure.second, ilvl + 6);
        }
        std::cout << std::string(ilvl + 3, ' ') << "}" << std::endl;

        std::cout << std::string(ilvl + 3, ' ') << "Functs={" << std::endl;
        for (auto function : module_desc.functions)
        {
            std::cout << std::string(ilvl + 6, ' ');
            std::cout << function.first << ":";
            print_func_desc(function.second, ilvl + 6);
        }
        std::cout << std::string(ilvl + 3, ' ') << "}" << std::endl;

        std::cout << ")" << std::endl;
    }

    void print_use_info(opt::use_info info)
    {
        std::cout << "UseInfo(";

        if (info.live)
            std::cout << "Live ";
        else
            std::cout << "Dead ";

        if (info.next_use == opt::NO_NEXT_USE)
            std::cout << "NoUse)";
        else
            std::cout << info.next_use << ")";
    }

    void print_icode_use_info(opt::icode_use_info& icode_use_info, int ilvl)
    {
        std::cout << "IcodeUseInfo=[" << std::endl;

        int i = 0;
        for (auto map : icode_use_info)
        {
            std::cout << std::string(ilvl + 3, ' ') << "e" << i << "={" << std::endl;

            for (auto pair : map)
            {
                std::cout << std::string(ilvl + 6, ' ');
                print_operand(pair.first);
                std::cout << ":";
                print_use_info(pair.second);
                std::cout << std::endl;
            }
            std::cout << std::string(ilvl + 3, ' ') << "}" << std::endl;

            i++;
        }

        std::cout << std::string(ilvl, ' ') << "]" << std::endl;
    }

    void print_use_map(opt::use_map& use_map, int ilvl)
    {
        std::cout << "UseMap={" << std::endl;

        for (auto parent_pair : use_map)
        {
            std::cout << std::string(ilvl + 3, ' ');
            std::cout << parent_pair.first << ":{" << std::endl;

            for (auto child_pair : parent_pair.second)
            {
                std::cout << std::string(ilvl + 6, ' ');
                std::cout << child_pair.first << ":";

                print_icode_use_info(child_pair.second, ilvl + 6);
            }

            std::cout << std::string(ilvl + 3, ' ') << "}" << std::endl;
        }

        std::cout << std::string(ilvl, ' ') << "}" << std::endl;
    }

    void print_vm(vm::uhllvm& vm)
    {
        int count = 0;

        for (vm::instruction instr : vm.instr_mem)
        {
            /* Print opcode */
            std::cout << count << "\t" << vm_instruction_strs[instr.opc] << " ";

            /* Print operand */
            switch (instr.opc)
            {
                /* No print */
                case vm::GBLMODE:
                case vm::FPMODE:
                case vm::CASTfi:
                case vm::CASTfui:
                case vm::CASTif:
                case vm::CASTiui:
                case vm::CASTuii:
                case vm::CASTuif:
                case vm::NOT:
                case vm::NEGf:
                case vm::NEGi:
                case vm::NEGui:
                case vm::PRINTf:
                case vm::PRINTi:
                case vm::PRINTui:
                case vm::INPUTi:
                case vm::INPUTf:
                case vm::INPUTui:
                case vm::SPACE:
                case vm::NEWLN:
                case vm::EXIT:
                    break;
                /* Int operand */
                case vm::LDIMi:
                case vm::MULIMi:
                case vm::DIVIMi:
                case vm::MODIMi:
                case vm::ADDIMi:
                case vm::SUBIMi:
                case vm::LTIMi:
                case vm::GTIMi:
                case vm::EQIMi:
                    std::cout << instr.opr.i;
                    break;
                /* Float operand */
                case vm::LDIMf:
                case vm::MULIMf:
                case vm::DIVIMf:
                case vm::MODIMf:
                case vm::ADDIMf:
                case vm::SUBIMf:
                case vm::LTIMf:
                case vm::GTIMf:
                case vm::EQIMf:
                    std::cout << instr.opr.f;
                /* Rest is unsinged int */
                default:
                    std::cout << instr.opr.ui;
                    break;
            }

            std::cout << std::endl;
            count += 1;
        }
    }

} // namespace log