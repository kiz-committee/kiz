/**
 * @file repl.cpp
 * @brief 交互式终端（Read Evaluate Print Loop，简称 REPL）核心实现
 *
 * @author azhz1107cat
 * @date 2025-10-25
 */

#include "repl.hpp"

#include "../lexer/lexer.hpp"
#include "../vm/vm.hpp"
#include "../ir_gen/ir_gen.hpp"
#include "../parser/parser.hpp"
#include "../kiz.hpp"
#include "color.hpp"
#include "../util/src_manager.hpp"

namespace ui {

const std::string Repl::file_path = "<shell#>";

Repl::Repl(): is_running_(true), multiline_start_(1), vm_(file_path) {
    std::cout << "This is the kiz REPL " << KIZ_VERSION << "\n" << std::endl;
}

std::string Repl::read(const std::string& prompt) {
    std::cout << Color::BRIGHT_MAGENTA << prompt << Color::RESET;
    std::cout.flush();
    std::string result = ui::get_whole_input(&std::cin, &std::cout);
    return result;
}

void Repl::loop() {
    DEBUG_OUTPUT("start repl loop");
    while (is_running_) {
        try {
            auto code = read(">>> "); // code 可能为多行
            DEBUG_OUTPUT("loop got input: " << code);
            auto old_code_iterator = err::SrcManager::opened_files.find(file_path);
            if (old_code_iterator != nullptr) {
                err::SrcManager::opened_files[file_path] = old_code_iterator->second + "\n" + code;
            } else {
                err::SrcManager::opened_files[file_path] = code;
            }
            handle_user_input(code);
            // eval_and_print(code);
        } catch (...) {}
    }
}

void Repl::eval_and_print(const std::string& cmd, const size_t startline) {
    DEBUG_OUTPUT("repl eval_and_print...");
    bool should_print = false;

    // init
    kiz::Lexer lexer(file_path);
    kiz::Parser parser(file_path);
    kiz::IRGenerator ir_gen(file_path);

    //auto lineno_start = get_history().size();
    auto lineno_start = startline;
    // 问题：原先REPL只支持一行输入，然后当前的行数恰恰就包含了那仅仅一条的新输入的语句，所以没有任何问题，但是现在支持了多行输入，应当从第一行多行输入的地方开始解析！
    const auto tokens = lexer.tokenize(cmd, lineno_start);

    auto ast = parser.parse(tokens);
    if (!ast->statements.empty() &&
        dynamic_cast<kiz::ExprStmt*>(ast->statements.back().get())
    )   should_print = true;

    const auto ir = ir_gen.gen(std::move(ast));
    if (vm_.call_stack.empty()) {
        const auto module = kiz::IRGenerator::gen_mod(file_path, ir);
        vm_.set_main_module(module);
    } else {
        assert(ir != nullptr && "No ir for run" );
        vm_.set_and_exec_curr_code(ir);
    }

    DEBUG_OUTPUT("repl print");
    auto stack_top = vm_.fetch_one_from_stack_top();
    if (stack_top != nullptr) {
        if (not dynamic_cast<model::Nil*>(stack_top) and should_print) {
            std::cout << stack_top->debug_string() << std::endl;
        }
    }
}

void Repl::handle_user_input(const std::string& cmd) {
    // add to history
    DEBUG_OUTPUT("Adding multiline_start_, now it is: " << multiline_start_ << ", cmd_history_.size(): " << cmd_history_.size());

    // 这里要计算实际的行数
    // 在末尾添加换行符以使得slice_file_content可以识别最后一行
    auto actual_additional_line_cnt = (err::slice_file_content(cmd + "\n")).size();
    DEBUG_OUTPUT("actual_additional_line_cnt: " << actual_additional_line_cnt);

    for (const auto &string_this : cmd_history_) {
        DEBUG_OUTPUT("curr: " << string_this);
    } // 似乎原来的cmd_history_永远视为一次一行，所以每一次行数加1

    for (const auto &per_cmd : cmd)
        cmd_history_.emplace_back(&per_cmd);

    // eval and print
    multiline_start_ += actual_additional_line_cnt;
    DEBUG_OUTPUT("After adding, multiline_start_: " << multiline_start_);
    eval_and_print(cmd, multiline_start_ - actual_additional_line_cnt);
    //因为抛出错误会导致后面代码不执行，所以应当先设定multiline_start_再减掉传给它
}

} // namespace repl