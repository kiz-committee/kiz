#include <tuple>

#include "../models/models.hpp"
#include "../kiz.hpp"
#include "vm.hpp"

namespace kiz {

auto Vm::fetch_two_from_stack_top(
    const std::string& op_name
)-> std::tuple<model::Object*, model::Object*> {
    if (op_stack.size() < 2) {
        std::string err_msg = "OP_" + op_name + ": 操作数栈元素不足（需≥2）";
        std::cout << err_msg << std::endl;
        assert(false);
    }
    // 栈顶是右操作数（后压入的），次顶是左操作数（先压入的）
    auto b = op_stack.top();
    op_stack.pop();
    auto a = op_stack.top();
    op_stack.pop();
    return {a, b};
}

// -------------------------- 算术指令 --------------------------
void Vm::exec_ADD(const Instruction& instruction) {
    DEBUG_OUTPUT("exec add...");
    auto [a, b] = fetch_two_from_stack_top("add");
    DEBUG_OUTPUT("a is " + a->debug_string() + ", b is " + b->debug_string());

    handle_call(get_attr(a, "__add__"), new model::List({b}), a);
    DEBUG_OUTPUT("success to call function");
}

void Vm::exec_SUB(const Instruction& instruction) {
    DEBUG_OUTPUT("exec sub...");
    auto [a, b] = fetch_two_from_stack_top("sub");

    handle_call(get_attr(a, "__sub__"), new model::List({b}), a);
}

void Vm::exec_MUL(const Instruction& instruction) {
    DEBUG_OUTPUT("exec mul...");
    auto [a, b] = fetch_two_from_stack_top("mul");

    handle_call(get_attr(a, "__mul__"), new model::List({b}), a);
}

void Vm::exec_DIV(const Instruction& instruction) {
    DEBUG_OUTPUT("exec div...");
    auto [a, b] = fetch_two_from_stack_top("div");

    handle_call(get_attr(a, "__div__"), new model::List({b}), a);
}

void Vm::exec_MOD(const Instruction& instruction) {
    DEBUG_OUTPUT("exec mod...");
    auto [a, b] = fetch_two_from_stack_top("mod");

    handle_call(get_attr(a, "__mod__"), new model::List({b}), a);

}

void Vm::exec_POW(const Instruction& instruction) {
    DEBUG_OUTPUT("exec pow...");
    auto [a, b] = fetch_two_from_stack_top("pow");

    handle_call(get_attr(a, "__pow__"), new model::List({b}), a);
}

void Vm::exec_NEG(const Instruction& instruction) {
    const auto raw_call_stack_count = call_stack.size();

    DEBUG_OUTPUT("exec neg...");
    auto a = op_stack.top();
    op_stack.pop();
    handle_call(get_attr(a, "__neg__"), new model::List({}), a);
}

// -------------------------- 比较指令 --------------------------
void Vm::exec_EQ(const Instruction& instruction) {
    DEBUG_OUTPUT("exec eq...");
    auto [a, b] = fetch_two_from_stack_top("eq");

    handle_call(get_attr(a, "__eq__"), new model::List({b}), a);
}

void Vm::exec_GT(const Instruction& instruction) {
    DEBUG_OUTPUT("exec gt...");
    auto [a, b] = fetch_two_from_stack_top("gt");

    handle_call(get_attr(a, "__gt__"), new model::List({b}), a);
}

void Vm::exec_LT(const Instruction& instruction) {
    DEBUG_OUTPUT("exec lt...");
    auto [a, b] = fetch_two_from_stack_top("lt");

    handle_call(get_attr(a, "__lt__"), new model::List({b}), a);
}

// -------------------------- 逻辑指令 --------------------------

// 修正NOT指令（逻辑取反+栈检查）
void Vm::exec_NOT(const Instruction& instruction) {
    DEBUG_OUTPUT("exec not...");
    // 栈检查：确保至少有1个操作数
    if (op_stack.empty()) {
        assert(false && "OP_NOT: 操作数栈元素不足（需≥1）");
    }
    // 弹出栈顶操作数
    auto a = op_stack.top();
    op_stack.pop();
    bool result = !is_true(a);
    op_stack.emplace(new model::Bool(result));
}

void Vm::exec_AND(const Instruction& instruction) {
    DEBUG_OUTPUT("exec and...");
    if (op_stack.size() < 2) {
        assert(false && "OP_AND: 操作数栈元素不足（需≥2）");
    }
    auto [a, b] = fetch_two_from_stack_top("and");

    if (!is_true(a)) {
        op_stack.emplace(a);
    } else {
        op_stack.emplace(b);
    }
}


void Vm::exec_OR(const Instruction& instruction) {
    DEBUG_OUTPUT("exec or...");
    if (op_stack.size() < 2) {
        assert(false && "OP_OR: 操作数栈元素不足（需≥2）");
    }
    auto [a, b] = fetch_two_from_stack_top("or");

    if (is_true(a)) {
        op_stack.emplace(a); // 压回原始对象a
    } else {
        op_stack.emplace(b);
    }
}

void Vm::exec_IS(const Instruction& instruction) {
    DEBUG_OUTPUT("exec is...");
    if (op_stack.size() < 2) {
        assert(false && "OP_IS: 操作数栈元素不足（需≥2）");
    }
    auto [a, b] = fetch_two_from_stack_top("is");

    bool is_same = a == b;
    auto* result = new model::Bool(is_same);
    op_stack.push(result);
}

void Vm::exec_GE(const Instruction& instruction) {
    auto [a, b] = fetch_two_from_stack_top("ge");
    call_function(get_attr(a, "__eq__"), new model::List({b}), a);
    call_function(get_attr(a, "__gt__"), new model::List({b}), a);
    auto [gt_result, eq_result] = fetch_two_from_stack_top("ge");

    auto result = new model::Bool(false);
    if (is_true(gt_result) or is_true(eq_result)) {
        result->val = true;
        op_stack.emplace(result);
        return;
    }
    op_stack.emplace(result);
}

void Vm::exec_LE(const Instruction& instruction) {
    auto [a, b] = fetch_two_from_stack_top("le");
    call_function(get_attr(a, "__eq__"), new model::List({b}), a);
    call_function(get_attr(a, "__lt__"), new model::List({b}), a);
    auto [lt_result, eq_result] = fetch_two_from_stack_top("le");

    auto result = new model::Bool(false);
    if (is_true(lt_result) or is_true(eq_result)) {
        result->val = true;
        op_stack.emplace(result);
        return;
    }
    op_stack.emplace(result);
}

void Vm::exec_NE(const Instruction& instruction) {
    auto [a, b] = fetch_two_from_stack_top("ne");
    call_function(get_attr(a, "__eq__"), new model::List({b}), a);
    if (is_true(op_stack.top())) {
        op_stack.pop();
        op_stack.emplace(new model::Bool(false));
        return;
    }
    op_stack.pop();
    op_stack.emplace(new model::Bool(true));
}

void Vm::exec_IN(const Instruction& instruction) {
    auto [item, for_check] = fetch_two_from_stack_top("contains");
    handle_call(get_attr(for_check, "contains"), new model::List({item}), for_check);
}

}
