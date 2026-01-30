/**
 * @file vm.hpp
 * @brief 虚拟机(VM)核心定义
 * 执行IR
 * @author azhz1107cat
 * @date 2025-10-25
 */
#pragma once

#include <cassert>

#include "../../deps/hashmap.hpp"

#include <stack>
#include <tuple>
#include <utility>

#include "../kiz.hpp"
#include "../error/error_reporter.hpp"

namespace model {
class Error;
}

namespace model {
class Module;
class CodeObject;
class Object;
}

namespace kiz {

enum class Opcode : uint8_t;

struct Instruction {
    Opcode opc;
    std::vector<size_t> opn_list;
    err::PositionInfo pos{};
    Instruction(Opcode o, std::vector<size_t> ol, err::PositionInfo& p) : opc(o), opn_list(std::move(ol)), pos(std::move(p)) {}
};

struct TryBlockInfo {
    size_t catch_start = 0;
};

struct CallFrame {
    std::string name;

    model::Object* owner;
    dep::HashMap<model::Object*> locals;

    size_t pc = 0;
    size_t return_to_pc;
    model::CodeObject* code_object;
    
    std::vector<TryBlockInfo> try_blocks;
};

class NativeFuncError final : public std::runtime_error {
public:
    std::string name;
    std::string msg;
    explicit NativeFuncError(std::string  name_, std::string  msg_) noexcept
        : name(std::move(name_)), msg(msg_), std::runtime_error(msg_) {}
};

class Vm {
public:
    static dep::HashMap<model::Module*> loaded_modules;
    static model::Module* main_module;

    static std::stack<model::Object*> op_stack;
    static std::vector<std::shared_ptr<CallFrame>> call_stack;

    static dep::HashMap<model::Object*> builtins;

    static bool running;
    static std::string file_path;

    static model::Error* curr_error;

    static dep::HashMap<model::Object*> std_modules;

    explicit Vm(const std::string& file_path_);

    static void entry_builtins();
    static void entry_std_modules();

    static void set_main_module(model::Module* src_module);
    static void exec_curr_code();
    static void set_and_exec_curr_code(const model::CodeObject* code_object);

    static void execute_instruction(const Instruction& instruction);

    static CallFrame* fetch_curr_call_frame();
    static model::Object* fetch_one_from_stack_top();
    static auto fetch_two_from_stack_top(const std::string& op_name)
        -> std::tuple<model::Object*, model::Object*>;

    static model::Object* get_attr(const model::Object* obj, const std::string& attr);
    static bool is_true(model::Object* obj);

    static void instruction_throw(const std::string& name, const std::string& content);
    static auto gen_pos_info()
        -> std::vector<std::pair<std::string, err::PositionInfo>>;
    static void handle_throw();

    /// 如果新增了调用栈，执行循环仅处理新增的模块栈帧（call_stack.size() > old_stack_size），不影响原有调用栈
    static void call_function(model::Object* func_obj, model::Object* args_obj, model::Object* self);

private:
    /// 如果用户函数则创建调用栈，如果内置函数则执行并压上返回值
    static void handle_call(model::Object* func_obj, model::Object* args_obj, model::Object* self);

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
    static void exec_GE(const Instruction& instruction);
    static void exec_LE(const Instruction& instruction);
    static void exec_NE(const Instruction& instruction);
    static void exec_AND(const Instruction& instruction);
    static void exec_NOT(const Instruction& instruction);
    static void exec_OR(const Instruction& instruction);
    static void exec_IS(const Instruction& instruction);

    static void exec_MAKE_LIST(const Instruction& instruction);
    static void exec_MAKE_DICT(const Instruction& instruction);
    static void exec_CALL(const Instruction& instruction);
    static void exec_RET(const Instruction& instruction);
    static void exec_GET_ATTR(const Instruction& instruction);
    static void exec_SET_ATTR(const Instruction& instruction);
    static void exec_GET_ITEM(const Instruction& instruction);
    static void exec_SET_ITEM(const Instruction& instruction);
    static void exec_CALL_METHOD(const Instruction& instruction);
    static void exec_LOAD_VAR(const Instruction& instruction);
    static void exec_LOAD_CONST(const Instruction& instruction);
    static void exec_SET_GLOBAL(const Instruction& instruction);
    static void exec_SET_LOCAL(const Instruction& instruction);
    static void exec_SET_NONLOCAL(const Instruction& instruction);

    static void exec_TRY_END(const Instruction& instruction);
    static void exec_TRY_START(const Instruction& instruction);
    static void exec_IMPORT(const Instruction& instruction);
    static void exec_LOAD_ERROR(const Instruction& instruction);

    static void exec_JUMP(const Instruction& instruction);
    static void exec_JUMP_IF_FALSE(const Instruction& instruction);
    static void exec_THROW(const Instruction& instruction);
    static void exec_IS_INSTANCE(const Instruction& instruction);
    static void exec_CREATE_OBJECT(const Instruction& instruction);
    static void exec_STOP(const Instruction& instruction);
};

} // namespace kiz