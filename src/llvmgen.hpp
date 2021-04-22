#ifndef LLVMGEN_HPP
#define LLVMGEN_HPP

#include <map>
#include <queue>

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"

#include "icode.hpp"
#include "log.hpp"

namespace llvmgen
{
    icode::target_desc target_desc();

    typedef std::pair<llvm::BasicBlock*, llvm::BasicBlock::iterator> llvm_bb_it_pair;
    typedef std::pair<size_t, icode::entry> entry_idx_pair;

    class llvm_generator
    {
        std::unique_ptr<llvm::LLVMContext> llvm_context;
        std::unique_ptr<llvm::Module> llvm_module;
        std::unique_ptr<llvm::IRBuilder<>> llvm_builder;

        std::map<std::string, llvm::AllocaInst*> alloca_inst_map;
        std::map<std::string, llvm::GlobalVariable*> llvm_global_map;
        std::map<std::string, llvm::Function*> llvm_function_map;
        std::map<icode::operand, llvm::Value*> operand_value_map;

        std::map<icode::operand, llvm::BasicBlock*> label_block_map;
        std::map<size_t, llvm::BasicBlock*> fall_block_map;
        std::queue<llvm::Value*> cmp_flag_q;
        std::map<size_t, llvm_bb_it_pair> backpatch_point_map;
        std::vector<entry_idx_pair> backpatch_entry_q;

        bool prev_instr_branch;

        icode::module_desc& module;

        llvm::Type* to_llvm_type(const icode::data_type dtype);
        llvm::Type* to_llvm_ptr_type(const icode::data_type dtype);
        llvm::Type* vinfo_to_llvm_type(const icode::var_info& var_info);

        llvm::Value* gen_ltrl(const icode::operand& op);
        llvm::Value* gen_addr(const icode::operand& op);

        llvm::Value* get_llvm_alloca(const icode::operand& op);
        llvm::Value* get_llvm_value(const icode::operand& op);
        void set_llvm_value(const icode::operand& op, llvm::Value* value);
        void local_symbol_alloca(icode::var_info& var_info, const std::string& name);
        void global_symbol_alloca(icode::var_info& var_info, const std::string& name);

        void create_ptr(const icode::entry& e);
        void eq(const icode::entry& e);
        void read(const icode::entry& e);
        void write(const icode::entry& e);
        void addrop(const icode::entry& e);

        llvm::Value* add(llvm::Value* LHS, llvm::Value* RHS, const icode::data_type dtype);
        llvm::Value* sub(llvm::Value* LHS, llvm::Value* RHS, const icode::data_type dtype);
        llvm::Value* mul(llvm::Value* LHS, llvm::Value* RHS, const icode::data_type dtype);
        llvm::Value* div(llvm::Value* LHS, llvm::Value* RHS, const icode::data_type dtype);
        llvm::Value* mod(llvm::Value* LHS, llvm::Value* RHS, const icode::data_type dtype);
        llvm::Value* rsh(llvm::Value* LHS, llvm::Value* RHS, const icode::data_type dtype);
        llvm::Value* lsh(llvm::Value* LHS, llvm::Value* RHS, const icode::data_type dtype);
        llvm::Value* bwa(llvm::Value* LHS, llvm::Value* RHS, const icode::data_type dtype);
        llvm::Value* bwo(llvm::Value* LHS, llvm::Value* RHS, const icode::data_type dtype);
        llvm::Value* bwx(llvm::Value* LHS, llvm::Value* RHS, const icode::data_type dtype);
        void binop(const icode::entry& e);

        llvm::Value* eq(llvm::Value* LHS, llvm::Value* RHS, const icode::data_type dtype);
        llvm::Value* neq(llvm::Value* LHS, llvm::Value* RHS, const icode::data_type dtype);
        llvm::Value* lt(llvm::Value* LHS, llvm::Value* RHS, const icode::data_type dtype);
        llvm::Value* lte(llvm::Value* LHS, llvm::Value* RHS, const icode::data_type dtype);
        llvm::Value* gt(llvm::Value* LHS, llvm::Value* RHS, const icode::data_type dtype);
        llvm::Value* gte(llvm::Value* LHS, llvm::Value* RHS, const icode::data_type dtype);
        void cmpop(const icode::entry& e, size_t entry_idx);

        void create_backpatch(const icode::entry& e, llvm::Function* F, size_t entry_idx);

        void create_label(const icode::entry& e, llvm::Function* F);

        void print(const icode::entry& e);

        void print_str(const icode::entry& e);

        void create_symbols(const icode::func_desc& func);

        void gen_func_icode(const icode::func_desc& func_desc, llvm::Function* F);

        void process_goto_backpatch();

        void gen_function(const icode::func_desc& func_desc, const std::string& name);

        void gen_globals();

        void setup_printf();

      public:
        llvm_generator(icode::module_desc& module_desc);
    };

}

#endif