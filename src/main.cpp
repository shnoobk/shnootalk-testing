#include <fstream>
#include <iostream>

#include "icode.hpp"
#include "irgen.hpp"
#include "lexer.hpp"
#include "llvmgen.hpp"
#include "log.hpp"
#include "parser.hpp"
#include "pathchk.hpp"
#include "token.hpp"

void print_usage()
{
    log::println("USAGE: uhllc MODULE [OPTION]");
    log::println("\nAvailable options:");
    log::println("\t-ast\tPrint parse tree");
    log::println("\t-ir\tPrint intermediate code representation");
    log::println("\t-asm\tPrint uhllvm assembly");
    log::println("\t-run\tExecute program (default)");
}

void ir_gen(const std::string& file_name,
            icode::target_desc& target,
            icode::module_desc_map& modules)
{
    /* Open file */
    std::ifstream ifile;
    ifile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    ifile.open(file_name + ".uhll");

    lexer::lexical_analyser lex(file_name, ifile);
    parser::rd_parser parse(lex, file_name, ifile);
    irgen::ir_generator gen(target, modules, file_name, ifile);

    /* Intermediate code generation */
    gen.initgen(parse.ast);

    /* Compile external modules that the module uses/imports */
    for (std::string use : modules[file_name].uses)
        if (modules.find(use) == modules.end())
            ir_gen(use, target, modules);

    /* Generate icode */
    gen.program(parse.ast);
}

int main(int argc, char* argv[])
{
    /* Check for correct usage */
    if (argc < 2 || argc > 3)
    {
        print_usage();
        return EXIT_FAILURE;
    }

    /* Get module name, if .uhll is present, strip it */
    std::string file_name = argv[1];

    std::string ext = ".uhll";
    if (file_name.size() > ext.size() &&
        file_name.substr(file_name.size() - ext.size()) == ext)
        file_name = file_name.substr(0, file_name.size() - ext.size());

    /* Get option passed by user, (if present) */
    std::string option;
    if (argc == 3)
    {
        option = argv[2];

        if (option != "-ir" && option != "-asm" && option != "-run" && option != "-ast")
        {
            print_usage();
            return EXIT_FAILURE;
        }
    }

    /* Map for holding all the compiled module in intermediate representation */
    icode::module_desc_map modules;

    /* Compile program */
    try
    {
        if (option == "-ast")
        {
            /* Open file */
            std::ifstream ifile;
            ifile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
            ifile.open(file_name + ".uhll");

            lexer::lexical_analyser lex(file_name, ifile);
            parser::rd_parser parse(lex, file_name, ifile);

            log::print_node(parse.ast);

            return 0;
        }

        icode::target_desc target = llvmgen::target_desc();

        ir_gen(file_name, target, modules);

        if (option == "-ir")
        {
            for (auto pair : modules)
            {
                log::print_module_desc(pair.second);
                log::println("");
            }

            return 0;
        }

        // uhllvmgen::uhllvm_generator vmgen(vm, modules, opt);

        // if (option == "-asm")
        //{
        //    log::print_vm(vm);
        //    return 0;
        //}
    }
    catch (const log::compile_error& e)
    {
        return EXIT_FAILURE;
    }
    catch (const log::internal_bug_error& e)
    {
        return EXIT_FAILURE;
    }
    catch (const std::ifstream::failure& e)
    {
        log::println("File I/O error");
        return EXIT_FAILURE;
    }

    /* Run program */
    // TODO

    return 0;
}