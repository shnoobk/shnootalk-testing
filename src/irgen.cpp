#include <algorithm>

#include "IRGenerator/DefineFromNode.hpp"
#include "IRGenerator/EnumFromNode.hpp"
#include "IRGenerator/VariableDescriptionFromNode.hpp"
#include "irgen.hpp"

namespace irgen
{
    ir_generator::ir_generator(icode::TargetDescription& target_desc,
                               icode::StringModulesMap& modules_map,
                               const std::string& file_name,
                               std::ifstream& ifile)

      : target(target_desc)
      , ext_modules_map(modules_map)
      , module(modules_map[file_name])
      , file(ifile)
      , builder(modules_map[file_name], ext_modules_map)
    {
        current_func_desc = nullptr;
        current_ext_module = &module;

        id_counter = 0;
        scope_id_counter = 0;

        module.name = file_name;
        module.initializeTargetInfo(target_desc);
    }

    unsigned int ir_generator::id()
    {
        return builder.id();
    }

    unsigned int ir_generator::get_scope_id()
    {
        return scope_id_stack.back();
    }

    void ir_generator::enter_scope()
    {
        scope_id_stack.push_back(++scope_id_counter);
    }

    void ir_generator::exit_scope()
    {
        scope_id_stack.pop_back();
    }

    void ir_generator::clear_scope()
    {
        scope_id_counter = 0;
        scope_id_stack.clear();
        scope_id_stack.push_back(0);
    }

    bool ir_generator::in_scope(unsigned int scope_id)
    {
        return std::find(scope_id_stack.begin(), scope_id_stack.end(), scope_id) != scope_id_stack.end();
    }

    void ir_generator::resetCurrentExternalModule()
    {
        current_ext_module = &module;
    }

    bool ir_generator::get_def(const std::string& name, icode::Define& def)
    {
        if ((*current_ext_module).getDefine(name, def))
            return true;

        if (module.getDefine(name, def))
            return true;

        return false;
    }

    bool ir_generator::get_func(const std::string& name, icode::FunctionDescription& func)
    {
        if ((*current_ext_module).getFunction(name, func))
            return true;

        if (module.getFunction(name, func))
            return true;

        return false;
    }

    bool ir_generator::get_enum(const std::string& name, int& val)
    {
        if (module.getEnum(name, val))
            return true;

        if ((*current_ext_module).getEnum(name, val))
            return true;

        return false;
    }

    std::pair<token::Token, icode::VariableDescription> ir_generator::var_from_node(const node::Node& root)
    {
        return variableDescriptionFromNode(*this, root);
    }

    void ir_generator::use(const node::Node& root)
    {
        for (node::Node child : root.children)
        {
            /* Get module name */
            token::Token name_token = child.tok;

            bool is_module = pathchk::file_exists(name_token.string + ".uhll");
            bool is_package = pathchk::dir_exists(name_token.string);

            /* Check if file exists */
            if (!(is_module || is_package))
            {
                miklog::error_tok(module.name, "Module or Package does not exist", file, name_token);
                throw miklog::compile_error();
            }

            /* Check for conflicts */
            if (is_module && is_package)
            {
                miklog::error_tok(module.name, "Module and Package exists with same name", file, name_token);
                throw miklog::compile_error();
            }

            /* Check for multiple imports */
            if (module.useExists(name_token.string))
            {
                miklog::error_tok(module.name, "Multiple imports detected", file, name_token);
                throw miklog::compile_error();
            }

            /* Check for name conflict */
            if (module.symbolExists(name_token.string))
            {
                miklog::error_tok(module.name, "Name conflict, symbol already exists", file, name_token);
                throw miklog::compile_error();
            }

            /* Check for self import */
            if (module.name == name_token.string)
            {
                miklog::error_tok(module.name, "Self import not allowed", file, name_token);
                throw miklog::compile_error();
            }

            /* Add to icode */
            module.uses.push_back(name_token.string);
        }
    }

    void ir_generator::from(const node::Node& root)
    {
        icode::StructDescription struct_desc;
        icode::FunctionDescription func_desc;
        icode::Define def;
        int enum_val;

        /* Get ext module */
        if (!module.useExists(root.children[0].tok.string))
        {
            miklog::error_tok(module.name, "Module not imported", file, root.children[0].tok);
            throw miklog::compile_error();
        }

        icode::ModuleDescription* ext_module = &ext_modules_map[root.children[0].tok.string];

        for (node::Node child : root.children[1].children)
        {
            /* Check if symbol exists */
            if (module.symbolExists(child.tok.string))
            {
                miklog::error_tok(module.name, "Symbol already defined in current module", file, child.tok);
                throw miklog::compile_error();
            }

            /* If it is struct */
            if ((*ext_module).getStruct(child.tok.string, struct_desc))
                module.structures[child.tok.string] = struct_desc;
            /* If it a function */
            else if ((*ext_module).getFunction(child.tok.string, func_desc))
            {
                miklog::error_tok(module.name, "Cannot import functions", file, child.tok);
                throw miklog::compile_error();
            }
            /* If is a def */
            else if ((*ext_module).getDefine(child.tok.string, def))
                module.defines[child.tok.string] = def;
            /* If it is a enum */
            else if ((*ext_module).getEnum(child.tok.string, enum_val))
                module.enumerations[child.tok.string] = enum_val;
            /* Check if use exists */
            else if ((*ext_module).useExists(child.tok.string))
                module.uses.push_back(child.tok.string);
            /* Does not exist */
            else
            {
                miklog::error_tok(module.name, "Symbol does not exist", file, child.tok);
                throw miklog::compile_error();
            }
        }
    }

    void ir_generator::enumeration(const node::Node& root)
    {
        enumFromNode(*this, root);
    }

    void ir_generator::def(const node::Node& root)
    {
        defineFromNode(*this, root);
    }

    void ir_generator::structure(const node::Node& root)
    {
        icode::StructDescription struct_desc;
        struct_desc.size = 0;

        token::Token name_token = root.children[0].tok;

        /* Check if symbol exists */
        if (module.symbolExists(name_token.string))
        {
            miklog::error_tok(module.name, "Symbol already defined", file, name_token);
            throw miklog::compile_error();
        }

        /* Go through field of structure */
        for (node::Node field : root.children[0].children)
        {
            std::pair<token::Token, icode::VariableDescription> var = var_from_node(field);

            /* Set mutable for var */
            var.second.setProperty(icode::IS_MUT);

            /* Check if the name is already a field */
            if (struct_desc.fieldExists(var.first.string))
            {
                miklog::error_tok(module.name, "Field already defined", file, var.first);
                throw miklog::compile_error();
            }

            if (module.symbolExists(var.first.string))
            {
                miklog::error_tok(module.name, "Symbol already defined", file, var.first);
                throw miklog::compile_error();
            }

            /* Update struct size and offset */
            var.second.offset = struct_desc.size;
            struct_desc.size += var.second.size;

            /* Append to feilds map */
            struct_desc.structFields[var.first.string] = var.second;

            /* Module name */
            struct_desc.moduleName = module.name;
        }

        /* Add strucuture definition to module */
        module.structures[name_token.string] = struct_desc;
    }

    void ir_generator::fn(const node::Node& root)
    {
        icode::FunctionDescription func_desc;
        std::string func_name;

        std::pair<token::Token, icode::VariableDescription> var = var_from_node(root);
        func_name = var.first.string;
        func_desc.functionReturnDescription = var.second;

        /* Check if function name symbol already exists */
        if (module.symbolExists(func_name))
        {
            miklog::error_tok(module.name, "Symbol already defined", file, root.children[0].tok);
            throw miklog::compile_error();
        }

        /* Process function parameters */
        for (size_t i = 1;; i++)
        {
            bool mut;
            if (root.children[i].type == node::PARAM)
                mut = false;
            else if (root.children[i].type == node::MUT_PARAM)
                mut = true;
            else
                break;

            std::pair<token::Token, icode::VariableDescription> param_var = var_from_node(root.children[i]);
            param_var.second.setProperty(icode::IS_PARAM);

            if (mut)
                param_var.second.setProperty(icode::IS_MUT);

            if (mut || param_var.second.dtype == icode::STRUCT || param_var.second.dimensions.size() != 0)
                param_var.second.setProperty(icode::IS_PTR);

            /* Check if symbol is already defined */
            if (module.symbolExists(param_var.first.string))
            {
                miklog::error_tok(module.name, "Symbol already defined", file, param_var.first);
                throw miklog::compile_error();
            }

            /* Append to symbol table */
            func_desc.parameters.push_back(param_var.first.string);
            func_desc.symbols[param_var.first.string] = param_var.second;
        }

        func_desc.moduleName = module.name;

        /* Add function definition to module */
        module.functions[func_name] = func_desc;
    }

    void ir_generator::global_var(const node::Node& root)
    {
        std::pair<token::Token, icode::VariableDescription> var = var_from_node(root);

        /* Set mut for var */
        var.second.setProperty(icode::IS_MUT);

        /* Check if symbol already exists */
        if (module.symbolExists(var.first.string))
        {
            miklog::error_tok(module.name, "Symbol already defined", file, var.first);
            throw miklog::compile_error();
        }

        /* Add to symbol table */
        module.globals[var.first.string] = var.second;
    }

    icode::Operand ir_generator::gen_str_dat(const token::Token& str_token, size_t char_count, icode::DataType dtype)
    {
        /* Append string data */
        std::string name = "_str_l" + std::to_string(str_token.line) + "_c" + std::to_string(str_token.column);
        module.stringsData[name] = str_token.unescapedString;

        /* Create icode::operand */
        size_t size = char_count * icode::getDataTypeSize(dtype);
        icode::Operand opr = icode::createStringDataOperand(name, size, id());

        return opr;
    }

    OperandDescriptionPair ir_generator::var_info_to_str_dat(const token::Token& str_token,
                                                             icode::VariableDescription var)
    {
        if (var.dimensions.size() != 1 || var.dtype != icode::UI8)
        {
            miklog::error_tok(module.name, "String assignment only allowed on 1D CHAR ARRAY", file, str_token);
            throw miklog::compile_error();
        }

        /* Check dimensions */
        size_t char_count = str_token.unescapedString.length();

        if (char_count > var.dimensions[0])
        {
            miklog::error_tok(module.name, "String too big", file, str_token);
            throw miklog::compile_error();
        }

        /* Create STR_DAT operand */
        icode::Operand opr = gen_str_dat(str_token, char_count, var.dtype);

        return OperandDescriptionPair(opr, var);
    }

    void ir_generator::assign_str_literal_tovar(OperandDescriptionPair var, node::Node& root)
    {
        if (var.second.dimensions.size() != 1 || var.second.dtype != icode::UI8)
        {
            miklog::error_tok(module.name, "String assignment only allowed on 1D INT ARRAY", file, root.tok);
            throw miklog::compile_error();
        }

        /* Check size */
        size_t char_count = root.tok.unescapedString.length();

        if (char_count > var.second.dimensions[0])
        {
            miklog::error_tok(module.name, "String too big", file, root.tok);
            throw miklog::compile_error();
        }

        /* Create Addr Temp */
        icode::Operand curr_offset = builder.createPointer(var.first);

        /* Loop through int and initialize string */
        for (size_t i = 0; i < char_count; i++)
        {
            char character = root.tok.unescapedString[i];

            /* Write to current offset */
            builder.copy(curr_offset, icode::createLiteralOperand(icode::UI8, character, id()));

            curr_offset =
              builder.addressAddOperator(curr_offset, icode::createLiteralAddressOperand(var.second.dtypeSize, id()));
        }

        /* Copy null character */
        builder.copy(curr_offset, icode::createLiteralOperand(icode::UI8, 0, id()));
    }

    void ir_generator::copy_array(icode::Operand& left, OperandDescriptionPair right)
    {
        icode::Operand curr_offset_left = builder.createPointer(left);
        icode::Operand curr_offset_right = builder.createPointer(right.first);

        unsigned int size = right.second.size;
        unsigned int dtype_size = right.second.dtypeSize;

        /* Loop through array and copy each element */
        for (size_t i = 0; i < size; i += dtype_size)
        {
            if (right.second.dtype == icode::STRUCT)
            {
                /* Copy struct from right to left */
                copy_struct(curr_offset_left, OperandDescriptionPair(curr_offset_right, right.second));
            }
            else
            {
                /* Copy element from right to left */
                builder.copy(curr_offset_left, curr_offset_right);
            }

            /* Update offset */
            if (i != size - dtype_size)
            {
                icode::Operand update = icode::createLiteralAddressOperand(dtype_size, id());
                curr_offset_left = builder.addressAddOperator(curr_offset_left, update);
                curr_offset_right = builder.addressAddOperator(curr_offset_right, update);
            }
        }
    }

    void ir_generator::copy_struct(icode::Operand& left, OperandDescriptionPair right)
    {
        icode::Operand curr_offset_left = builder.createPointer(left);
        icode::Operand curr_offset_right = builder.createPointer(right.first);

        /* Loop through each field and copy them */
        unsigned int count = 0;
        icode::Operand update;
        for (auto field : module.structures[right.second.dtypeName].structFields)
        {
            icode::VariableDescription field_info = field.second;

            if (count != 0)
            {
                curr_offset_left.updateDataType(field.second);
                curr_offset_right.updateDataType(field.second);

                curr_offset_left = builder.addressAddOperator(curr_offset_left, update);
                curr_offset_right = builder.addressAddOperator(curr_offset_right, update);
            }

            /* Copy field */
            if (field_info.dimensions.size() != 0)
            {
                copy_array(curr_offset_left, OperandDescriptionPair(curr_offset_right, field_info));
            }
            else if (field_info.dtype != icode::STRUCT)
            {
                /* Copy field from right into left */
                builder.copy(curr_offset_left, curr_offset_right);
            }
            else
            {
                copy_struct(curr_offset_left, OperandDescriptionPair(curr_offset_right, field_info));
            }

            update = icode::createLiteralAddressOperand(field_info.size, id());

            count++;
        }
    }

    void ir_generator::assign_init_list_tovar(OperandDescriptionPair var, node::Node& root)
    {
        /* Cannot use initializer list to assign to var */
        if (var.second.dimensions.size() == 0)
        {
            miklog::error_tok(module.name, "Cannot initialize a NON-ARRAY with initializer list", file, root.tok);
            throw miklog::compile_error();
        }

        icode::Operand curr_offset = builder.createPointer(var.first);

        /* Create var info for the elements inside the list */
        icode::VariableDescription element_var = var.second;
        element_var.size /= element_var.dimensions[0];
        element_var.dimensions.erase(element_var.dimensions.begin());

        /* Keep track of number of dimensions written */
        unsigned int dim_count = 0;

        for (size_t i = 0; i < root.children.size(); i++)
        {
            node::Node child = root.children[i];

            if (dim_count >= var.second.dimensions[0])
            {
                miklog::error_tok(module.name, "Dimension size too big", file, child.tok);
                throw miklog::compile_error();
            }

            if (element_var.dimensions.size() == 0)
            {
                /* Check if expression */
                if (child.type != node::TERM && child.type != node::EXPRESSION)
                {
                    miklog::error_tok(module.name, "Incorrect dimensions", file, child.tok);
                    throw miklog::compile_error();
                }

                OperandDescriptionPair element_expr = expression(child);

                /* Type check */
                if (!icode::isSameType(element_var, element_expr.second))
                {
                    miklog::type_error(module.name, file, child.tok, element_var, element_expr.second);
                    throw miklog::compile_error();
                }

                if (element_expr.second.dtype != icode::STRUCT)
                {
                    /* Write to current offset if not a struct */
                    builder.copy(curr_offset, element_expr.first);
                }
                else
                {
                    copy_struct(curr_offset, element_expr);
                }
            }
            else if (child.type == node::STR_LITERAL)
            {
                assign_str_literal_tovar(OperandDescriptionPair(curr_offset, element_var), child);
            }
            else
            {
                assign_init_list_tovar(OperandDescriptionPair(curr_offset, element_var), child);
            }

            dim_count++;

            /* Update offset */
            if (i != root.children.size() - 1)
            {
                curr_offset =
                  builder.addressAddOperator(curr_offset, icode::createLiteralAddressOperand(element_var.size, id()));
            }
        }

        if (dim_count < var.second.dimensions[0])
        {
            miklog::error_tok(module.name, "Dimension size too small", file, root.tok);
            throw miklog::compile_error();
        }
    }

    void ir_generator::var(const node::Node& root)
    {
        std::pair<token::Token, icode::VariableDescription> var = var_from_node(root);

        /* Set mutable for var */
        if (root.type == node::VAR)
            var.second.setProperty(icode::IS_MUT);

        /* Check if symbol already exists */
        if (module.symbolExists(var.first.string) || (*current_func_desc).symbolExists(var.first.string))
        {
            miklog::error_tok(module.name, "Symbol already defined", file, var.first);
            throw miklog::compile_error();
        }

        /* Check for initialization expression or initializer list */
        node::Node last_node = root.children.back();

        if (last_node.type == node::EXPRESSION || last_node.type == node::TERM)
        {
            /* If an array requires initializer list */
            if (var.second.dimensions.size() != 0)
            {
                miklog::error_tok(module.name, "Initializer list required to initialize array", file, last_node.tok);
                throw miklog::compile_error();
            }

            /* Create icode operands, one for variable other for temp
                to hold result of initialization expression */
            icode::Operand left =
              icode::createVarOperand(var.second.dtype, var.second.dtypeName, var.first.string, id());

            OperandDescriptionPair init_exp = expression(last_node);

            /* Check if the type match */
            if (!icode::isSameType(var.second, init_exp.second))
            {
                miklog::type_error(module.name, file, last_node.tok, var.second, init_exp.second);
                throw miklog::compile_error();
            }

            if (var.second.dtype != icode::STRUCT)
            {
                /* Create EQUAL icode entry is not a STRUCT */
                builder.copy(left, init_exp.first);
            }
            else
            {
                copy_struct(left, init_exp);
            }
        }
        else if (last_node.type == node::STR_LITERAL)
        {
            OperandDescriptionPair var_pair = OperandDescriptionPair(
              icode::createVarOperand(var.second.dtype, var.second.dtypeName, var.first.string, id()),
              var.second);
            assign_str_literal_tovar(var_pair, last_node);
        }
        else if (last_node.type == node::INITLIST)
        {
            OperandDescriptionPair var_pair = OperandDescriptionPair(
              icode::createVarOperand(var.second.dtype, var.second.dtypeName, var.first.string, id()),
              var.second);
            assign_init_list_tovar(var_pair, last_node);
        }

        /* Add to symbol table */
        (*current_func_desc).symbols[var.first.string] = var.second;
    }

    OperandDescriptionPair ir_generator::var_access(const node::Node& root)
    {
        icode::Operand current_offset_temp;
        icode::VariableDescription current_var_info;
        std::string ident_name;
        unsigned int dim_count = 0;
        unsigned int rem_dim = 0;
        bool is_global = false;
        bool is_ptr = false;

        int enum_val;
        icode::Define def;

        /* Check if identifier exists and get dtype and size */
        node::Node child = root.children[0];
        ident_name = child.tok.string;
        if ((*current_func_desc).getSymbol(ident_name, current_var_info))
        {
            is_ptr = current_var_info.checkProperty(icode::IS_PTR);
        }
        else if (module.getGlobal(ident_name, current_var_info))
        {
            is_global = true;
        }
        else if (get_enum(ident_name, enum_val))
        {
            /* Check if there is STRUCT or ARRAY access of enum */
            if (root.children.size() > 1)
            {
                miklog::error_tok(module.name, "Invalid use of ENUM", file, child.tok);
                throw miklog::compile_error();
            }

            icode::Operand op = icode::createLiteralOperand(icode::INT, enum_val, id());
            icode::VariableDescription var = icode::variableDescriptionFromDataType(icode::INT, target);
            return OperandDescriptionPair(op, var);
        }
        else if (get_def(ident_name, def))
        {
            /* Check if there is STRUCT or ARRAY access of def */
            if (root.children.size() > 1)
            {
                miklog::error_tok(module.name, "Invalid use of DEF", file, child.tok);
                throw miklog::compile_error();
            }

            icode::Operand op;

            if (def.dtype == icode::INT)
                op = icode::createLiteralOperand(icode::INT, def.val.integer, id());
            else
                op = icode::createLiteralOperand(icode::FLOAT, def.val.floating, id());

            icode::VariableDescription var = icode::variableDescriptionFromDataType(def.dtype, target);
            return OperandDescriptionPair(op, var);
        }
        else
        {
            miklog::error_tok(module.name, "Symbol does not exist", file, child.tok);
            throw miklog::compile_error();
        }

        /* Check if the variable is available in the current scope */
        if (!in_scope(current_var_info.scopeId))
        {
            miklog::error_tok(module.name, "Symbol not in scope", file, child.tok);
            throw miklog::compile_error();
        }

        /* If no struct or subscript */
        if (root.children.size() == 1)
        {
            icode::Operand op = icode::createVarOperand(current_var_info.dtype,
                                                        current_var_info.dtypeName,
                                                        ident_name,
                                                        id(),
                                                        is_global,
                                                        is_ptr);
            return OperandDescriptionPair(op, current_var_info);
        }

        current_offset_temp = icode::createVarOperand(current_var_info.dtype,
                                                      current_var_info.dtypeName,
                                                      ident_name,
                                                      id(),
                                                      is_global,
                                                      is_ptr);

        /* Ensure current_offset_temp is a pointer */
        current_offset_temp = builder.createPointer(current_offset_temp);

        /* Go through struct fields and subsripts */
        for (size_t i = 1; i < root.children.size();)
        {
            node::Node child = root.children[i];

            switch (child.type)
            {
                case node::STRUCT_VAR:
                {
                    child = root.children[++i];

                    if (current_var_info.dtype != icode::STRUCT)
                    {
                        miklog::error_tok(module.name, "STRUCT access on a NON-STRUCT data type", file, child.tok);
                        throw miklog::compile_error();
                    }
                    else if (dim_count == 0 && current_var_info.dimensions.size() != 0)
                    {
                        miklog::error_tok(module.name, "STRUCT access on an ARRAY", file, child.tok);
                        throw miklog::compile_error();
                    }
                    else
                    {
                        /* Get struct description */
                        icode::StructDescription struct_desc;
                        struct_desc =
                          ext_modules_map[current_var_info.moduleName].structures[current_var_info.dtypeName];

                        /* Check if field exists */
                        if (struct_desc.fieldExists(child.tok.string))
                        {
                            /* If it does, update offset */
                            bool is_mut = current_var_info.checkProperty(icode::IS_MUT);

                            /* Update var info to struct field */
                            current_var_info = struct_desc.structFields[child.tok.string];

                            /* If the struct is immutable, fields are automatically
                             * immutable */
                            if (!is_mut)
                                current_var_info.clearProperty(icode::IS_MUT);

                            /* Update pointer dtype */
                            current_offset_temp.updateDataType(current_var_info);

                            /* Add offset */
                            current_offset_temp = builder.addressAddOperator(
                              current_offset_temp,
                              icode::createLiteralAddressOperand(current_var_info.offset, id()));

                            dim_count = 0;
                        }
                        else
                        {
                            miklog::error_tok(module.name, "Undefined STRUCT field", file, child.tok);
                            throw miklog::compile_error();
                        }
                    }
                    i++;
                    break;
                }
                case node::SUBSCRIPT:
                {
                    dim_count = 1;

                    if (current_var_info.dimensions.size() == 0)
                    {
                        miklog::error_tok(module.name, "ARRAY access on a NON ARRAY", file, child.tok);
                        throw miklog::compile_error();
                    }

                    unsigned int elem_width = current_var_info.size / current_var_info.dimensions[0];

                    while (child.type == node::SUBSCRIPT)
                    {
                        if (dim_count > current_var_info.dimensions.size())
                        {
                            miklog::error_tok(module.name, "Too many subscripts", file, child.tok);
                            throw miklog::compile_error();
                        }

                        /* Operand to store result of expression */
                        OperandDescriptionPair subs_expr = expression(child.children[0]);

                        /* Check if int expression */
                        if (!icode::isInteger(subs_expr.second.dtype) || subs_expr.second.dimensions.size() != 0)
                        {
                            miklog::error_tok(module.name, "Index must be an integer", file, child.children[0].tok);
                            throw miklog::compile_error();
                        }

                        /* Entry for subscript expression */
                        icode::Operand subs_op =
                          builder.addressMultiplyOperator(subs_expr.first,
                                                          icode::createLiteralAddressOperand(elem_width, id()));

                        if (dim_count != current_var_info.dimensions.size())
                            elem_width /= current_var_info.dimensions[dim_count];

                        /* Create entry for adding to the current offset */
                        current_offset_temp = builder.addressAddOperator(current_offset_temp, subs_op);

                        if (i < root.children.size() - 1)
                        {
                            child = root.children[++i];
                            if (child.type == node::SUBSCRIPT)
                                dim_count++;
                        }
                        else
                        {
                            i++;
                            break;
                        };
                    }

                    /* After indexing, only some dimensions will be left */
                    rem_dim = current_var_info.dimensions.size() - dim_count;

                    current_var_info.dimensions.erase(current_var_info.dimensions.begin(),
                                                      current_var_info.dimensions.end() - rem_dim);

                    break;
                }
                default:
                {
                    miklog::internal_error_tok(module.name, file, child.tok);
                    throw miklog::internal_bug_error();
                }
            }
        }

        /* Return var */
        return OperandDescriptionPair(current_offset_temp, current_var_info);
    }

    OperandDescriptionPair ir_generator::funccall(const node::Node& root)
    {
        icode::ModuleDescription* temp = current_ext_module;

        /* Get the first argument (if not a string literal) */
        OperandDescriptionPair first_arg;
        if (root.children.size() != 0)
        {
            if (root.children[0].type != node::STR_LITERAL)
                first_arg = expression(root.children[0]);

            /* If struct funccall, switch to struct's (first arg's) module */
            if (root.type == node::STRUCT_FUNCCALL)
                current_ext_module = &ext_modules_map[first_arg.second.moduleName];
        }

        /* Check if function exits */
        icode::FunctionDescription func_desc;
        std::string func_name = root.tok.string;

        if (!get_func(func_name, func_desc))
        {
            miklog::error_tok(module.name, "Function does not exist", file, root.tok);
            throw miklog::compile_error();
        }

        /* Check number of parameters */
        if (root.children.size() != func_desc.parameters.size())
        {
            miklog::error_tok(module.name, "Number of parameters don't match", file, root.tok);
            throw miklog::compile_error();
        }

        /* Go through each parameter */
        for (size_t i = 0; i < root.children.size(); i++)
        {
            /* Get parameter information */
            icode::VariableDescription param = func_desc.symbols[func_desc.parameters[i]];
            bool mut = param.checkProperty(icode::IS_MUT);

            /* Get argument passed to function */
            OperandDescriptionPair arg;
            if (root.children[i].type == node::STR_LITERAL)
            {
                arg = var_info_to_str_dat(root.children[i].tok, param);
            }
            else if (i == 0 && root.children[0].type != node::STR_LITERAL)
                arg = first_arg;
            else
                arg = expression(root.children[i]);

            /* Type check */
            if (!icode::isSameType(param, arg.second))
            {
                miklog::type_error(module.name, file, root.children[i].tok, param, arg.second);
                throw miklog::compile_error();
            }

            /* If mutable, it has to be a variable or address */
            if (mut && (arg.first.operandType == icode::TEMP || arg.first.operandType == icode::STR_DATA ||
                        arg.first.operandType == icode::LITERAL))
            {
                miklog::error_tok(module.name,
                                  "Cannot pass an EXPRESSION or STRING LITERAL as MUTABLE",
                                  file,
                                  root.children[i].tok);
                throw miklog::compile_error();
            }

            /* If param is mut, arg has to be mutable to */
            if (mut && !arg.second.checkProperty(icode::IS_MUT))
            {
                miklog::error_tok(module.name, "Cannot pass IMMUTABLE as MUTABLE", file, root.children[i].tok);
                throw miklog::compile_error();
            }

            /* Pass arguments */

            /* If mut or struct or array, pass by ref */
            if (mut || param.dtype == icode::STRUCT || param.dimensions.size() != 0)
                builder.pass(icode::PASS_ADDR, arg.first, func_name, func_desc);
            /* Else pass by value */
            else
                builder.pass(icode::PASS, arg.first, func_name, func_desc);
        }

        /* Call function */
        icode::Operand ret_temp = builder.call(func_name, func_desc);

        /* Switch back to current module if ext_mod was modified */
        if (root.type == node::STRUCT_FUNCCALL)
            current_ext_module = temp;

        return OperandDescriptionPair(ret_temp, func_desc.functionReturnDescription);
    }

    OperandDescriptionPair ir_generator::size_of(const node::Node& root)
    {
        std::string ident = root.children.back().tok.string;

        /* Enter the specified ext module */
        icode::ModuleDescription* current_module = &module;

        node::Node mod_node = root.children[0];

        int i = 0;
        while (mod_node.type == node::MODULE)
        {
            std::string mod_name = mod_node.tok.string;

            /* Check if module exists */
            if (!(*current_module).useExists(mod_name))
            {
                miklog::error_tok(module.name, "Module does not exist", file, mod_node.tok);
                throw miklog::compile_error();
            }

            /* Swtich to module */
            current_module = &ext_modules_map[mod_name];

            i++;
            mod_node = root.children[i];
        }

        /* Get size of type */
        int size = 0;
        icode::DataType dtype = module.dataTypeFromString(ident);

        icode::StructDescription struct_desc;
        icode::VariableDescription global;
        icode::VariableDescription symbol;

        if (dtype != icode::STRUCT)
            size = icode::getDataTypeSize(dtype);
        else if ((*current_module).getStruct(ident, struct_desc))
            size = struct_desc.size;
        else if ((*current_module).getGlobal(ident, global))
            size = global.size;
        else if ((*current_func_desc).getSymbol(ident, symbol))
            size = symbol.dtypeSize;
        else
        {
            miklog::error_tok(module.name, "Symbol not found", file, root.tok);
            throw miklog::compile_error();
        }

        /* return a icode::INT literal  */
        return OperandDescriptionPair(icode::createLiteralOperand(icode::INT, size, id()),
                                      icode::variableDescriptionFromDataType(icode::INT, target));
    }

    OperandDescriptionPair ir_generator::term(const node::Node& root)
    {
        node::Node child = root.children[0];
        switch (child.type)
        {
            case node::LITERAL:
            {
                switch (child.tok.type)
                {
                    case token::INT_LITERAL:
                    case token::HEX_LITERAL:
                    case token::BIN_LITERAL:
                    {
                        /* Return literal icode operand */
                        int literal = std::stoi(child.tok.string);
                        icode::DataType dtype = icode::INT;
                        return OperandDescriptionPair(icode::createLiteralOperand(dtype, literal, id()),
                                                      icode::variableDescriptionFromDataType(dtype, target));

                        break;
                    }
                    case token::CHAR_LITERAL:
                    {
                        char c = child.tok.unescapedString[0];

                        icode::DataType dtype = icode::UI8;
                        return OperandDescriptionPair(icode::createLiteralOperand(dtype, c, id()),
                                                      icode::variableDescriptionFromDataType(dtype, target));
                    }
                    case token::FLOAT_LITERAL:
                    {
                        /* Return literal icode operand */
                        icode::DataType dtype = icode::FLOAT;
                        float literal = (float)stof(child.tok.string);
                        return OperandDescriptionPair(icode::createLiteralOperand(dtype, literal, id()),
                                                      icode::variableDescriptionFromDataType(dtype, target));

                        break;
                    }
                    default:
                    {
                        miklog::internal_error_tok(module.name, file, child.tok);
                        throw miklog::internal_bug_error();
                    }
                }
            }
            case node::IDENTIFIER:
            {
                return var_access(root);
            }
            case node::CAST:
            {
                icode::DataType cast_dtype = module.dataTypeFromString(child.tok.string);

                OperandDescriptionPair cast_term = term(child.children[0]);

                /* Cannot cast ARRAY */
                if (cast_term.second.dimensions.size() != 0 || cast_term.second.dtype == icode::STRUCT)
                {
                    miklog::error_tok(module.name, "Cannot cast STRUCT or ARRAY", file, child.tok);
                    throw miklog::compile_error();
                }

                /* Create icode entry for casting */
                icode::Operand res_temp = builder.castOperator(cast_dtype, cast_term.first);

                /* Return temp */
                return OperandDescriptionPair(res_temp, icode::variableDescriptionFromDataType(cast_dtype, target));
            }
            case node::UNARY_OPR:
            {
                OperandDescriptionPair term_var = term(child.children[0]);

                icode::DataType dtype = term_var.second.dtype;
                std::string dtype_name = term_var.second.dtypeName;

                /* Unary operator not allowed on ARRAY */
                if (term_var.second.dimensions.size() != 0)
                {
                    miklog::error_tok(module.name, "Unary operator not allowed on ARRAY", file, child.tok);
                    throw miklog::compile_error();
                }

                /* Unary operator not allowed on STRUCT */
                if (dtype == icode::STRUCT)
                {
                    miklog::error_tok(module.name, "Unary operator not allowed on STRUCT", file, child.tok);
                    throw miklog::compile_error();
                }

                /* NOT operator not allowed on float */
                if (!icode::isInteger(dtype) && child.tok.type == token::NOT)
                {
                    miklog::error_tok(module.name, "Unary operator NOT not allowed on FLOAT", file, child.tok);
                    throw miklog::compile_error();
                }

                icode::Instruction opcode;
                switch (child.tok.type)
                {
                    case token::MINUS:
                        opcode = icode::UNARY_MINUS;
                        break;
                    case token::NOT:
                        opcode = icode::NOT;
                        break;
                    case token::CONDN_NOT:
                        miklog::error_tok(module.name, "Did not expect CONDN NOT", file, child.tok);
                        throw miklog::compile_error();
                    default:
                        miklog::internal_error_tok(module.name, file, child.tok);
                        throw miklog::internal_bug_error();
                }

                icode::Operand res_temp =
                  builder.unaryOperator(opcode, icode::createTempOperand(dtype, dtype_name, id()), term_var.first);

                /* Return temp */
                return OperandDescriptionPair(res_temp, term_var.second);
            }
            case node::EXPRESSION:
            {
                return expression(child);
            }
            case node::STRUCT_FUNCCALL:
            case node::FUNCCALL:
            {
                return funccall(child);
            }
            case node::MODULE:
            {
                /* Enter the specified ext module */
                icode::ModuleDescription* current_module = &module;

                node::Node mod_node = root.children[0];

                int i = 0;
                while (mod_node.type == node::MODULE)
                {
                    std::string mod_name = mod_node.tok.string;

                    /* Check if module exists */
                    if (!(*current_module).useExists(mod_name))
                    {
                        miklog::error_tok(module.name, "Module does not exist", file, mod_node.tok);
                        throw miklog::compile_error();
                    }

                    /* Swtich to module */
                    current_module = &ext_modules_map[mod_name];

                    i++;
                    mod_node = root.children[i];
                }

                if (root.children[i].tok.type != token::IDENTIFIER)
                {
                    miklog::error_tok(module.name, "Invalid use of MODULE ACCESS", file, child.tok);
                    throw miklog::compile_error();
                }

                current_ext_module = current_module;

                OperandDescriptionPair ret_val = term(root.children[i]);

                current_ext_module = &module;

                return ret_val;
            }
            case node::SIZEOF:
            {
                return size_of(child);
            }
            default:
            {
                miklog::internal_error_tok(module.name, file, child.tok);
                throw miklog::internal_bug_error();
            }
        }

        return OperandDescriptionPair(icode::Operand(), icode::variableDescriptionFromDataType(icode::VOID, target));
    }

    icode::Instruction ir_generator::tokenToBinaryOperator(const token::Token tok)
    {
        switch (tok.type)
        {
            case token::MULTIPLY:
                return icode::MUL;
            case token::DIVIDE:
                return icode::DIV;
            case token::MOD:
                return icode::MOD;
            case token::PLUS:
                return icode::ADD;
            case token::MINUS:
                return icode::SUB;
            case token::RIGHT_SHIFT:
                return icode::RSH;
            case token::LEFT_SHIFT:
                return icode::LSH;
            case token::BITWISE_AND:
                return icode::BWA;
            case token::BITWISE_XOR:
                return icode::BWX;
            case token::BITWISE_OR:
                return icode::BWO;
            case token::CONDN_AND:
            case token::CONDN_OR:
            case token::LESS_THAN:
            case token::LESS_THAN_EQUAL:
            case token::GREATER_THAN:
            case token::GREATER_THAN_EQUAL:
            case token::CONDN_EQUAL:
            case token::CONDN_NOT_EQUAL:
                miklog::error_tok(module.name, "Did not expect conditional operator", file, tok);
                throw miklog::compile_error();
            default:
                miklog::internal_error_tok(module.name, file, tok);
                throw miklog::internal_bug_error();
        }
    }

    OperandDescriptionPair ir_generator::expression(const node::Node& root)
    {
        if (root.type == node::TERM)
            return term(root);

        if (root.children.size() == 1)
            return expression(root.children[0]);

        token::Token expr_opr = root.children[1].tok;

        /* First operand */
        OperandDescriptionPair first_operand = expression(root.children[0]);
        icode::DataType dtype = first_operand.second.dtype;
        std::string dtype_name = first_operand.second.dtypeName;

        /* Expression not allowed on arrays or struct */
        if (dtype == icode::STRUCT || first_operand.second.dimensions.size() != 0)
        {
            miklog::error_tok(module.name, "Operator not allowed on STRUCT or ARRAY", file, expr_opr);
            throw miklog::compile_error();
        }

        /* Second operand */
        OperandDescriptionPair second_operand = expression(root.children[2]);

        /* Type check */
        if (!icode::isSameType(first_operand.second, second_operand.second))
        {
            miklog::type_error(module.name, file, root.children[2].tok, first_operand.second, second_operand.second);
            throw miklog::compile_error();
        }

        /* If bitwise operator, dtype has to be int */
        if (expr_opr.isBitwiseOperation() && !icode::isInteger(dtype))
        {
            miklog::error_tok(module.name, "Bitwise operations not allowed on FLOAT", file, expr_opr);
            throw miklog::compile_error();
        }

        /* Generate corresponding opcode for operator */
        icode::Instruction opcode = tokenToBinaryOperator(expr_opr);

        icode::Operand res_temp = builder.binaryOperator(opcode,
                                                         icode::createTempOperand(dtype, dtype_name, id()),
                                                         first_operand.first,
                                                         second_operand.first);

        /* Return the operand where final result is stored */
        return OperandDescriptionPair(res_temp, first_operand.second);
    }

    icode::Instruction ir_generator::assignmentTokenToBinaryOperator(const token::Token tok)
    {
        /* Convert token type to opcode */
        switch (tok.type)
        {
            case token::EQUAL:
                return icode::EQUAL;
            case token::PLUS_EQUAL:
                return icode::ADD;
            case token::MINUS_EQUAL:
                return icode::SUB;
            case token::DIVIDE_EQUAL:
                return icode::DIV;
            case token::MULTIPLY_EQUAL:
                return icode::MUL;
            case token::OR_EQUAL:
                return icode::BWO;
            case token::AND_EQUAL:
                return icode::BWA;
            case token::XOR_EQUAL:
                return icode::BWX;
            default:
                miklog::internal_error_tok(module.name, file, tok);
                throw miklog::internal_bug_error();
        }
    }

    void ir_generator::assignment(const node::Node& root)
    {
        /* The variable to write to */
        OperandDescriptionPair var = var_access(root.children[0]);

        /* Assignment expression */
        OperandDescriptionPair expr = expression(root.children[2]);

        /* Assignement operator */
        token::Token assign_opr = root.children[1].tok;

        /* Var connot be enum of a def */
        if (var.first.operandType == icode::LITERAL)
        {
            miklog::error_tok(module.name, "Cannot assign to LITERAL", file, root.children[0].tok);
            throw miklog::compile_error();
        }

        /* Check if type matches */
        if (!icode::isSameType(var.second, expr.second))
        {
            miklog::type_error(module.name, file, root.children[2].tok, var.second, expr.second);
            throw miklog::compile_error();
        }

        /* Check if mutable */
        if (!var.second.checkProperty(icode::IS_MUT))
        {
            miklog::error_tok(module.name, "Cannot modify IMMUTABLE variable or parameter", file, root.children[0].tok);
            throw miklog::compile_error();
        }

        /* Assignment operator not allowed on arrays */
        if (var.second.dimensions.size() != 0)
        {
            miklog::error_tok(module.name, "Assignment operators not allowed on ARRAY", file, assign_opr);
            throw miklog::compile_error();
        }

        /* Only EQUAL operator allowed on STRUCT */
        if (var.second.dtype == icode::STRUCT && assign_opr.type != token::EQUAL)
        {
            miklog::error_tok(module.name, "Only EQUAL operator allowed on STRUCT", file, assign_opr);
            throw miklog::compile_error();
        }

        /* If bitwise operator, dtype has to be int */
        if (assign_opr.isBitwiseOperation() && !icode::isInteger(var.second.dtype))
        {
            miklog::error_tok(module.name, "Bitwise operation not allowed on FLOAT", file, assign_opr);
            throw miklog::compile_error();
        }

        /* Create icode entry */
        icode::Instruction opcode = assignmentTokenToBinaryOperator(assign_opr);

        /* If struct */
        if (var.second.dtype == icode::STRUCT)
        {
            copy_struct(var.first, expr);
            return;
        }

        /* If not a struct field */
        if (assign_opr.type == token::EQUAL)
        {
            builder.copy(var.first, expr.first);
        }
        else
        {
            icode::Operand temp = icode::createTempOperand(var.second.dtype, var.second.dtypeName, id());
            builder.copy(temp, var.first);
            builder.binaryOperator(opcode, var.first, temp, expr.first);
        }
    }

    icode::Operand ir_generator::gen_label(token::Token tok, bool true_label, std::string prefix)
    {
        /* Generate label using token's line and col number */

        std::string label_name = "_l" + std::to_string(tok.line) + "_c" + std::to_string(tok.column);

        if (true_label)
            return icode::createLabelOperand("_" + prefix + "_true" + label_name, id());

        return icode::createLabelOperand("_" + prefix + "_false" + label_name, id());
    }

    icode::Instruction ir_generator::tokenToCompareOperator(const token::Token tok)
    {
        switch (tok.type)
        {
            case token::LESS_THAN:
                return icode::LT;
            case token::LESS_THAN_EQUAL:
                return icode::LTE;
            case token::GREATER_THAN:
                return icode::GT;
            case token::GREATER_THAN_EQUAL:
                return icode::GTE;
            case token::CONDN_EQUAL:
                return icode::EQ;
            case token::CONDN_NOT_EQUAL:
                return icode::NEQ;
            default:
                miklog::error_tok(module.name, "Invalid conditional expression", file, tok);
                throw miklog::compile_error();
        }
    }

    void ir_generator::condn_expression(const node::Node& root,
                                        const icode::Operand& t_label,
                                        const icode::Operand& f_label,
                                        bool t_fall,
                                        bool f_fall)
    {
        if (root.type == node::TERM)
        {
            /* For unary operator NOT */
            if (root.children[0].type == node::EXPRESSION)
            {
                condn_expression(root.children[0], t_label, f_label, t_fall, f_fall);
                return;
            }

            if (root.children[0].tok.type != token::CONDN_NOT)
            {
                miklog::error_tok(module.name, "Invalid conditional expression", file, root.tok);
                throw miklog::compile_error();
            }

            condn_expression(root.children[0].children[0].children[0], f_label, t_label, f_fall, t_fall);
        }
        else if (root.children.size() == 1)
        {
            condn_expression(root.children[0], t_label, f_label, t_fall, f_fall);
        }
        else
        {
            token::Token expr_opr = root.children[1].tok;

            /* See the dragon book, Figure 6.39 and Figure 6.40 */

            switch (expr_opr.type)
            {
                case token::CONDN_AND:
                {
                    icode::Operand new_t_label = gen_label(expr_opr, true);
                    icode::Operand new_f_label = gen_label(expr_opr, false);

                    if (!f_fall)
                        condn_expression(root.children[0], new_t_label, f_label, true, false);
                    else
                        condn_expression(root.children[0], new_t_label, new_f_label, true, false);

                    condn_expression(root.children[2], t_label, f_label, t_fall, f_fall);

                    if (f_fall)
                    {
                        builder.label(new_f_label);
                    }

                    break;
                }
                case token::CONDN_OR:
                {
                    icode::Operand new_t_label = gen_label(expr_opr, true);
                    icode::Operand new_f_label = gen_label(expr_opr, false);

                    if (!t_fall)
                        condn_expression(root.children[0], t_label, new_f_label, false, true);
                    else
                        condn_expression(root.children[0], new_t_label, new_f_label, false, true);

                    condn_expression(root.children[2], t_label, f_label, t_fall, f_fall);

                    if (t_fall)
                    {
                        builder.label(new_t_label);
                    }

                    break;
                }
                default:
                {
                    icode::Instruction opcode = tokenToCompareOperator(expr_opr);

                    /* Create icode entry for comparing two expressions */
                    OperandDescriptionPair first_operand = expression(root.children[0]);

                    /* Cannot compare structs and arrays */
                    if (first_operand.second.dtype == icode::STRUCT || first_operand.second.dimensions.size() != 0)
                    {
                        miklog::error_tok(module.name, "Cannot compare STRUCT or ARRAYS", file, expr_opr);
                        throw miklog::compile_error();
                    }

                    OperandDescriptionPair second_operand = expression(root.children[2]);

                    /* Type check */
                    if (!icode::isSameType(first_operand.second, second_operand.second))
                    {
                        miklog::type_error(module.name,
                                           file,
                                           root.children[2].tok,
                                           first_operand.second,
                                           second_operand.second);
                        throw miklog::compile_error();
                    }

                    /* If second operand is a ptr, read it into a temp */
                    builder.compareOperator(opcode, first_operand.first, second_operand.first);

                    /* Create icode entry for goto */

                    if (!t_fall)
                    {
                        builder.createBranch(icode::IF_TRUE_GOTO, t_label);
                    }

                    if (!f_fall)
                    {
                        builder.createBranch(icode::IF_FALSE_GOTO, f_label);
                    }
                }
            }
        }
    }

    void ir_generator::ifstmt(const node::Node& root,
                              bool loop,
                              const icode::Operand& start_label,
                              const icode::Operand& break_label,
                              const icode::Operand& cont_label)
    {
        /* Create label for end of all if statements */
        icode::Operand end_label = gen_label(root.tok, false, "ifend");

        for (size_t i = 0; i < root.children.size(); i++)
        {
            node::Node child = root.children[i];

            icode::Operand new_t_label = gen_label(child.tok, true, "if");
            icode::Operand new_f_label = gen_label(child.tok, false, "if");

            if (child.type != node::ELSE)
            {
                /* Process conditional expression */
                condn_expression(child.children[0], new_t_label, new_f_label, true, false);

                /* Process block */
                block(child.children[1], loop, start_label, break_label, cont_label);

                if (i != root.children.size() - 1)
                {
                    /* Go to end */
                    builder.createBranch(icode::GOTO, end_label);
                }

                /* Create label to skip block */
                builder.label(new_f_label);
            }
            else
            {
                block(child.children[0], loop, start_label, break_label, cont_label);
            }
        }

        if (root.children.size() != 1)
        {
            /* Create label for end of if statement */
            builder.label(end_label);
        }
    }

    void ir_generator::whileloop(const node::Node& root)
    {
        icode::Operand new_t_label = gen_label(root.tok, true, "while");
        icode::Operand new_f_label = gen_label(root.tok, false, "while");

        /* Create label for looping */
        builder.label(new_t_label);

        /* Process conditional expression */
        condn_expression(root.children[0], new_t_label, new_f_label, true, false);

        /* Process block */
        block(root.children[1], true, new_t_label, new_f_label, new_t_label);

        /* Go back to beginning */
        builder.createBranch(icode::GOTO, new_t_label);

        /* Create label to skip block */
        builder.label(new_f_label);
    }

    void ir_generator::forloop(const node::Node& root)
    {
        /* Process initialization */
        if (root.children[0].type == node::VAR)
            var(root.children[0]);
        else
            assignment(root.children[0]);

        /* Process conditional  */
        icode::Operand new_t_label = gen_label(root.tok, true, "for");
        icode::Operand new_f_label = gen_label(root.tok, false, "for");
        icode::Operand cont_label = gen_label(root.tok, true, "for_cont");

        /* Create label for looping */
        builder.label(new_t_label);

        /* Process conditional expression */
        condn_expression(root.children[1], new_t_label, new_f_label, true, false);

        /* Process block */
        block(root.children[3], true, new_t_label, new_f_label, cont_label);

        /* Create label for continue */
        builder.label(cont_label);

        /* Process assignment */
        assignment(root.children[2]);

        /* Go back to beginning */
        builder.createBranch(icode::GOTO, new_t_label);

        /* Create label to skip block */
        builder.label(new_f_label);
    }

    void ir_generator::print(const node::Node& root)
    {
        for (size_t i = 0; i < root.children.size(); i++)
        {
            node::Node child = root.children[i];

            /* If string literal, create temp string an print it */
            if (child.type == node::STR_LITERAL)
            {
                /* Get str len and str size */
                int char_count = child.tok.unescapedString.length();

                icode::Operand str_dat_opr = gen_str_dat(child.tok, char_count, icode::UI8);

                builder.printOperator(icode::PRINT_STR, str_dat_opr);
            }
            /* Else expression */
            else
            {
                OperandDescriptionPair print_var = expression(child);

                /* Cannot peint struct or arrays */
                if (print_var.second.dtype == icode::STRUCT || print_var.second.dimensions.size() > 1)
                {
                    miklog::error_tok(module.name, "Cannot print STRUCT or multi-dimensional ARRAY", file, child.tok);
                    throw miklog::compile_error();
                }

                if (print_var.second.dimensions.size() != 0)
                    builder.printOperator(icode::PRINT_STR, print_var.first);
                else
                    builder.printOperator(icode::PRINT, print_var.first);
            }

            /* Add space after printing (except for the last print) */
            if (i != root.children.size() - 1)
                builder.noArgumentEntry(icode::SPACE);

            /* If println, the last print should add new line */
            if (i == root.children.size() - 1 && root.type == node::PRINTLN)
                builder.noArgumentEntry(icode::NEWLN);
        }
    }

    void ir_generator::input(const node::Node& root)
    {
        OperandDescriptionPair input_var = expression(root.children[0]);

        /* Check if the input var is writable */
        if (!(input_var.first.operandType == icode::VAR || input_var.first.operandType == icode::GBL_VAR ||
              input_var.first.operandType == icode::TEMP_PTR || input_var.first.operandType == icode::PTR))
        {
            miklog::error_tok(module.name, "Invalid term for INPUT", file, root.children[0].tok);
            throw miklog::compile_error();
        }

        if (input_var.second.dtype == icode::STRUCT)
        {
            miklog::error_tok(module.name, "Cannot INPUT STRUCT", file, root.children[0].tok);
            throw miklog::compile_error();
        }

        if (input_var.second.dimensions.size() > 1)
        {
            miklog::error_tok(module.name, "Cannot INPUT more than 1D ARRAY", file, root.children[0].tok);
            throw miklog::compile_error();
        }

        if (input_var.second.dimensions.size() == 1 && !icode::isInteger(input_var.first.dtype))
        {
            miklog::error_tok(module.name, "String input requires 1D INT ARRAY", file, root.children[0].tok);
            throw miklog::compile_error();
        }

        /* Create INPUT or INPUT_STR entry */
        if (input_var.second.dimensions.size() == 0)
            builder.inputOperator(icode::INPUT, input_var.first);
        else
            builder.inputOperator(icode::INPUT_STR, input_var.first, input_var.second.dimensions[0]);
    }

    void ir_generator::block(const node::Node& root,
                             bool loop,
                             const icode::Operand& start_label,
                             const icode::Operand& break_label,
                             const icode::Operand& cont_label)
    {
        /* Setup scope */
        enter_scope();

        for (node::Node stmt : root.children)
        {
            switch (stmt.type)
            {
                case node::VAR:
                case node::CONST:
                    var(stmt);
                    break;
                case node::ASSIGNMENT:
                    assignment(stmt);
                    break;
                case node::ASSIGNMENT_STR:
                {
                    /* The variable to write to */
                    OperandDescriptionPair var = var_access(stmt.children[0]);

                    assign_str_literal_tovar(var, stmt.children[1]);

                    break;
                }
                case node::ASSIGNMENT_INITLIST:
                {
                    /* The variable to write to */
                    OperandDescriptionPair var = var_access(stmt.children[0]);

                    assign_init_list_tovar(var, stmt.children[1]);

                    break;
                }
                case node::FUNCCALL:
                    funccall(stmt);
                    break;
                case node::TERM:
                    funccall(stmt.children[0]);
                    break;
                case node::MODULE:
                {
                    /* Check if the module exists */
                    if (!(*current_ext_module).useExists(stmt.tok.string))
                    {
                        miklog::error_tok(module.name, "Module does not exist", file, stmt.tok);
                        throw miklog::compile_error();
                    }

                    /* Switch to external module */
                    icode::ModuleDescription* temp = current_ext_module;
                    current_ext_module = &ext_modules_map[stmt.tok.string];

                    OperandDescriptionPair ret_val = funccall(stmt.children[0]);

                    /* Switch back to self */
                    current_ext_module = temp;

                    break;
                }
                case node::IF:
                    ifstmt(stmt, loop, start_label, break_label, cont_label);
                    break;
                case node::WHILE:
                    whileloop(stmt);
                    break;
                case node::FOR:
                    forloop(stmt);
                    break;
                case node::BREAK:
                {
                    if (!loop)
                    {
                        miklog::error_tok(module.name, "BREAK outside loop", file, stmt.tok);
                        throw miklog::compile_error();
                    }

                    /* Go to end */
                    builder.createBranch(icode::GOTO, break_label);

                    break;
                }
                case node::CONTINUE:
                {
                    if (!loop)
                    {
                        miklog::error_tok(module.name, "CONTINUE outside loop", file, stmt.tok);
                        throw miklog::compile_error();
                    }

                    /* Go to end */
                    builder.createBranch(icode::GOTO, cont_label);

                    break;
                }
                case node::RETURN:
                {
                    icode::VariableDescription ret_info = (*current_func_desc).functionReturnDescription;

                    /* Get return value */
                    if (stmt.children.size() != 0)
                    {
                        OperandDescriptionPair ret_val = expression(stmt.children[0]);

                        /* Type check */
                        if (!icode::isSameType(ret_info, ret_val.second))
                        {
                            miklog::type_error(module.name, file, stmt.children[0].tok, ret_info, ret_val.second);
                            throw miklog::compile_error();
                        }

                        /* Assign return value to return pointer */
                        icode::Operand ret_ptr =
                          icode::createRetPointerOperand(ret_info.dtype, ret_info.dtypeName, id());

                        if (ret_val.second.dtype == icode::STRUCT)
                            copy_struct(ret_ptr, ret_val);
                        else
                        {
                            builder.copy(ret_ptr, ret_val.first);
                        }
                    }
                    else if (ret_info.dtype != icode::VOID)
                    {
                        miklog::error_tok(module.name, "Ret type is not VOID", file, stmt.tok);
                        throw miklog::compile_error();
                    }

                    /* Add return to icode */
                    builder.noArgumentEntry(icode::RET);

                    break;
                }
                case node::PRINT:
                case node::PRINTLN:
                    print(stmt);
                    break;
                case node::INPUT:
                    input(stmt);
                    break;
                case node::EXIT:
                {
                    builder.noArgumentEntry(icode::EXIT);
                    break;
                }
                default:
                    miklog::internal_error_tok(module.name, file, stmt.tok);
                    throw miklog::internal_bug_error();
            }
        }

        exit_scope();
    }

    void ir_generator::initgen(const node::Node& root)
    {
        /* Get uses */
        for (node::Node child : root.children)
        {
            if (child.type == node::USE)
                use(child);
            else
                break;
        }
    }

    bool ir_generator::current_function_terminates()
    {
        if ((*current_func_desc).icodeTable.size() < 1)
            return false;

        icode::Instruction last_opcode = (*current_func_desc).icodeTable.back().opcode;

        if (last_opcode == icode::RET)
            return true;

        return false;
    }

    void ir_generator::program(const node::Node& root)
    {
        /* Setup scope */
        clear_scope();

        /* Build symbol table */
        for (node::Node child : root.children)
        {
            switch (child.type)
            {
                case node::USE:
                    break;
                case node::FROM:
                    from(child);
                    break;
                case node::STRUCT:
                    structure(child);
                    break;
                case node::FUNCTION:
                    fn(child);
                    break;
                case node::ENUM:
                    enumeration(child);
                    break;
                case node::DEF:
                    def(child);
                    break;
                case node::VAR:
                    global_var(child);
                    break;
                default:
                    miklog::internal_error_tok(module.name, file, child.tok);
                    throw miklog::internal_bug_error();
            }
        }

        /* Process function blocks */
        for (node::Node child : root.children)
        {
            if (child.type == node::FUNCTION)
            {
                /* Get function name */
                std::string func_name = child.children[0].tok.string;

                /* Switch symbol and icode table */
                current_func_desc = &module.functions[func_name];
                builder.setFunctionDescription(current_func_desc);

                /* Clear scope */
                clear_scope();

                /* Process block */
                block(child.children.back(),
                      false,
                      icode::createLabelOperand("", 0),
                      icode::createLabelOperand("", 0),
                      icode::createLabelOperand("", 0));

                /* Last instruction must be return */
                if (!current_function_terminates())
                {
                    if (current_func_desc->functionReturnDescription.dtype != icode::VOID)
                    {
                        miklog::error_tok(module.name, "Missing RETURN for this FUNCTION", file, child.tok);
                        throw miklog::compile_error();
                    }

                    builder.noArgumentEntry(icode::RET);
                }
            }
        }
    }
} // namespace irgen
