/**
 * @file wasm_interface.cpp
 * @brief WASM接口文件 - 简化版本
 *
 * 提供JavaScript可以调用的C接口
 */

#include "ir_gen/ir_gen.hpp"
#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "error/src_manager.hpp"
#include "vm/vm.hpp"

#include <string>
#include <cstring>

extern "C" {
    // 运行代码并返回结果
    const char* run_code(const char* code, int length) {
        static std::string result;
        static std::string current_path = "<playground#>";

        try {
            std::string content(code, length);
            kiz::Lexer lexer(current_path);
            kiz::Parser parser(current_path);
            kiz::IRGenerator ir_gen(current_path);

            lexer.prepare(content, 1);
            const auto tokens = lexer.tokenize();
            auto ast = parser.parse(tokens);
            const auto ir = ir_gen.gen(std::move(ast));
            const auto module = kiz::IRGenerator::gen_mod(current_path, ir);

            kiz::Vm::set_main_module(module);
            kiz::Vm::exec_curr_code();
            kiz::Vm::handle_ensure();

            result = "Success";
        } catch (const KizStopRunningSignal& e) {
            result = std::string("A Panic! : ") + e.what();
        } catch (...) {
            result = "Unknown error";
        }

        return result.c_str();
    }
}