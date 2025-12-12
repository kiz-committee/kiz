/**
 * @file vm.hpp
 * @brief 虚拟机(VM)核心定义
 * 执行IR
 * @author azhz1107cat
 * @date 2025-10-25
 */
#pragma once

#include "../deps/hashmap.hpp"

#include <stack>
#include <tuple>

#include "kiz.hpp"

namespace model {
class Object;
class Module;
class CodeObject;
}

namespace kiz {

enum class Opcode;


struct Instruction {
    Opcode opc;
    std::vector<size_t> opn_list;
    size_t start_lineno;
    size_t end_lineno;
};


struct VmState{
    model::Object* stack_top;
    deps::HashMap<model::Object*> locals;
};

struct CallFrame {
    bool is_week_scope;
    deps::HashMap<model::Object*> locals;
    size_t pc = 0;
    size_t return_to_pc;
    std::string name;
    model::CodeObject* code_object;
    std::vector<std::tuple<size_t, size_t>> curr_lineno_map;
    std::vector<std::string> names;
};

class Vm {
    static deps::HashMap<model::Module*> loaded_modules;
    static model::Module* main_module;
    static std::stack<model::Object*> op_stack_;
    static std::vector<std::unique_ptr<CallFrame>> call_stack_;
    static bool running_;
    static std::string file_path;
public:
    static deps::HashMap<model::Object*> builtins;

    explicit Vm(const std::string& file_path_);
    static void load(model::Module* src_module);
    static void extend_code(const model::CodeObject* code_object);
    static void load_required_modules(const deps::HashMap<model::Module*>& modules);
    static VmState get_vm_state();
    static void exec(const Instruction& instruction);
    static std::tuple<model::Object*, model::Object*> fetch_two_from_stack_top(const std::string& curr_instruction_name);

    static model::Object* get_attr(const model::Object* obj, const std::string& attr);
    static std::string get_obj_to_str(const model::Object* obj);
    static std::string get_obj_debug_str(const model::Object* obj);
    static bool check_obj_is_true(model::Object* obj);
    static void call_function(model::Object* func_obj, model::Object* args_obj, model::Object* self);

private:
    static void exec_ADD(const Instruction& instruction);
    static void exec_SUB(const Instruction& instruction);
    static void exec_MUL(const Instruction& instruction);
    static void exec_DIV(const Instruction& instruction);
    static void exec_MOD(const Instruction& instruction);
    static void exec_POW(const Instruction& instruction);
    static void exec_NEG(const Instruction& instruction);
    static void exec_EQ(const Instruction& instruction);
    static void exec_GT(const Instruction& instruction);
    static void exec_LT(const Instruction& instruction);
    static void exec_AND(const Instruction& instruction);
    static void exec_NOT(const Instruction& instruction);
    static void exec_OR(const Instruction& instruction);
    static void exec_IS(const Instruction& instruction);
    static void exec_IN(const Instruction& instruction);
    static void exec_MAKE_LIST(const Instruction& instruction);
    static void exec_CALL(const Instruction& instruction);
    static void exec_RET(const Instruction& instruction);
    static void exec_GET_ATTR(const Instruction& instruction);
    static void exec_SET_ATTR(const Instruction& instruction);
    static void exec_CALL_METHOD(const Instruction& instruction);
    static void exec_LOAD_VAR(const Instruction& instruction);
    static void exec_LOAD_CONST(const Instruction& instruction);
    static void exec_SET_GLOBAL(const Instruction& instruction);
    static void exec_SET_LOCAL(const Instruction& instruction);
    static void exec_SET_NONLOCAL(const Instruction& instruction);
    static void exec_JUMP(const Instruction& instruction);
    static void exec_JUMP_IF_FALSE(const Instruction& instruction);
    static void exec_THROW(const Instruction& instruction);
    static void exec_POP_TOP(const Instruction& instruction);
    static void exec_SWAP(const Instruction& instruction);
    static void exec_COPY_TOP(const Instruction& instruction);
    static void exec_STOP(const Instruction& instruction);
};

} // namespace kiz