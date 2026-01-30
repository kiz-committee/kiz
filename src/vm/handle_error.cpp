#include <ranges>

#include "vm.hpp"
#include "builtins/include/builtin_functions.hpp"

namespace kiz {

// -------------------------- 异常处理 --------------------------
auto Vm::gen_pos_info() -> std::vector<std::pair<std::string, err::PositionInfo>> {
    size_t frame_index = 0;
    std::vector<std::pair<std::string, err::PositionInfo>> positions;
    std::string path;
    for (const auto& frame: call_stack) {
        if (const auto m = dynamic_cast<model::Module*>(frame->owner)) {
            path = m->path;
        }
        err::PositionInfo pos;
        bool cond = frame_index == call_stack.size() - 1;
        DEBUG_OUTPUT("frame_index: " << frame_index << ", call_stack.size(): " << call_stack.size());
        if (cond) {
            pos = frame->code_object->code.at(frame->pc).pos;
        } else {
            pos = frame->code_object->code.at(frame->pc - 1).pos;
        }
        DEBUG_OUTPUT(
            "Vm::gen_pos_info, pos = col "
            << pos.col_start << ", " << pos.col_end << " | line "
            << pos.lno_start << ", " << pos.lno_end
        );
        positions.emplace_back(path, pos);
        ++frame_index;
    }
    return positions;
}

void Vm::instruction_throw(const std::string& name, const std::string& content) {
    const auto err_name = new model::String(name);
    const auto err_msg = new model::String(content);

    const auto err_obj = new model::Error();
    err_obj->positions = gen_pos_info();
    err_obj->attrs.insert("__name__", err_name);
    err_obj->attrs.insert("__msg__", err_msg);
    DEBUG_OUTPUT("err_obj pos size = "+std::to_string(err_obj->positions.size()));
    curr_error = err_obj;
    handle_throw();
}


// 辅助函数
std::pair<std::string, std::string> get_err_name_and_msg(const model::Object* err_obj) {
    assert(err_obj != nullptr);
    auto err_name_it = err_obj->attrs.find("__name__");
    auto err_msg_it = err_obj->attrs.find("__msg__");
    assert(err_name_it != nullptr);
    assert(err_msg_it != nullptr);
    auto err_name = err_name_it->value->debug_string();
    auto err_msg = err_msg_it->value->debug_string();
    return {err_name, err_msg};
}

void Vm::handle_throw() {
    assert(curr_error != nullptr);

    size_t frames_to_pop = 0;
    CallFrame* target_frame = nullptr;
    size_t catch_pc = 0;

    // 逆序遍历调用栈，寻找最近的 try 块（且当前不在该 catch 块内）
    for (auto frame_it = call_stack.rbegin(); frame_it != call_stack.rend(); ++frame_it) {
        CallFrame* frame = (*frame_it).get();
        if (!frame->try_blocks.empty()) {
            target_frame = frame;
            catch_pc = frame->try_blocks.back().catch_start;
            assert(catch_pc != 0);
            std::cout << "find catch pc!" << catch_pc << std::endl;
            break;
        }
        frames_to_pop++;
    }

    // 如果找到有效的 try 块
    if (target_frame) {
        // 弹出多余的栈帧
        for (size_t i = 0; i < frames_to_pop; ++i) {
            call_stack.pop_back();
        }
        // 设置 pc 到 catch 块开始
        target_frame->pc = catch_pc;
        return;
    }

    auto [error_name, error_msg] = get_err_name_and_msg(curr_error);

    // 报错
    std::cout << Color::BRIGHT_RED << "\nTrace Back: " << Color::RESET << std::endl;
    DEBUG_OUTPUT("curr err pos size: "+std::to_string(curr_error->positions.size()));
    for (auto& [_path, _pos]: curr_error->positions ) {
        DEBUG_OUTPUT(_path + " " + std::to_string(_pos.lno_start) + " "
            + std::to_string(_pos.lno_end) + " " + std::to_string(_pos.col_start) + " " + std::to_string(_pos.col_end));
        err::context_printer(_path, _pos);
    }

    DEBUG_OUTPUT(error_name+" "+error_msg);

    // 错误信息（类型加粗红 + 内容白）
    std::cout << Color::BOLD << Color::BRIGHT_RED << error_name
              << Color::RESET << Color::WHITE << " : " << error_msg
              << Color::RESET << std::endl;
    std::cout << std::endl;

    throw KizStopRunningSignal();
}

void Vm::exec_ENTER_TRY(const Instruction& instruction) {
    assert(!call_stack.empty() && "exec_ENTER_TRY: 调用栈为空，无法执行ENTER_TRY指令");
    size_t catch_start = instruction.opn_list[0];
    size_t finally_start = instruction.opn_list[1];
    call_stack.back()->try_blocks.emplace_back(catch_start, finally_start);
}

void Vm::exec_POP_TRY_FRAME(const Instruction& instruction) {
    assert(!call_stack.empty());
    call_stack.back()->try_blocks.pop_back();
}


void Vm::exec_LOAD_ERROR(const Instruction& instruction) {
    DEBUG_OUTPUT("loading curr error" + curr_error->debug_string());
    assert(curr_error != nullptr);
    op_stack.push(curr_error);
}

void Vm::exec_THROW(const Instruction& instruction) {
    DEBUG_OUTPUT("exec throw...");
    auto* top = dynamic_cast<model::Error*>(op_stack.top());
    assert(top != nullptr);
    top->positions = gen_pos_info();
    curr_error = top;
    op_stack.pop();

    handle_throw();
}


void Vm::exec_IS_INSTANCE(const Instruction& instruction) {
    auto b = fetch_one_from_stack_top();
    auto a = fetch_one_from_stack_top();
    op_stack.emplace(builtin::check_based_object(a, b));
}

}