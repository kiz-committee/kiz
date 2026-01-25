#include "parser.hpp"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <memory>
#include <vector>

#include "../kiz.hpp"
#include "../repl/color.hpp"

namespace kiz {

// 需要end结尾的块
std::unique_ptr<BlockStmt> Parser::parse_block(TokenType endswith) {
    DEBUG_OUTPUT("parsing block (with end)");
    std::vector<std::unique_ptr<Stmt>> block_stmts;
    auto block_tok = curr_token();

    while (curr_tok_idx_ < tokens_.size()) {
        const Token& curr_tok = curr_token();

        if (curr_tok.type == endswith or curr_tok.type == TokenType::End) {
            break;
        }

        if (curr_tok.type == TokenType::EndOfFile) {
            // Original: assert(false && "Block not terminated with 'end'");
            // Now:
            err::error_reporter(this -> file_path, curr_token().pos, "ParsingError", "Block not terminated with 'end'");
        }

        if (auto stmt = parse_stmt()) {
            block_stmts.push_back(std::move(stmt));
        }
    }

    return std::make_unique<BlockStmt>(block_tok.pos, std::move(block_stmts));
}

// parse_if实现
std::unique_ptr<IfStmt> Parser::parse_if() {
    DEBUG_OUTPUT("parsing if");
    // 解析if条件表达式
    auto cond_expr = parse_expression();
    // Original: assert(cond_expr!=nullptr && "Invalid if condition");
    // Now:
    if (cond_expr == nullptr)
        err::error_reporter(this -> file_path,
            curr_token().pos,
            "SyntaxError",
            "Invalid if condition"
        );

    // 解析if体（无end的块）
    skip_start_of_block();
    auto if_tok = curr_token();
    auto if_block = parse_block(TokenType::Else);

    // 处理else分支
    std::unique_ptr<BlockStmt> else_block = nullptr;
    if (curr_token().type == TokenType::Else) {
        skip_token("else");
        skip_start_of_block();
        if (curr_token().type == TokenType::If) {
            // else if分支
            std::vector<std::unique_ptr<Stmt>> else_if_stmts;
            else_if_stmts.push_back(parse_stmt());
            else_block = std::make_unique<BlockStmt>(curr_token().pos, std::move(else_if_stmts));
        } else {
            // else分支（无end的块）
            else_block = parse_block();
        }
    }

    if (curr_token().type == TokenType::End) {
        skip_token("end");
    }

    return std::make_unique<IfStmt>(if_tok.pos, std::move(cond_expr), std::move(if_block), std::move(else_block));
}

// parse_stmt实现
std::unique_ptr<Stmt> Parser::parse_stmt() {
    DEBUG_OUTPUT("parsing stmt");
    const Token curr_tok = curr_token();

    // 解析if语句
    if (curr_tok.type == TokenType::If) {
        skip_token("if");
        return parse_if();  // 复用parse_if逻辑
    }

    // 解析while语句（适配end结尾）
    if (curr_tok.type == TokenType::While) {
        DEBUG_OUTPUT("parsing while");
        auto tok = skip_token("while");
        // 解析循环条件表达式
        auto cond_expr = parse_expression();
        if (cond_expr == nullptr)
            err::error_reporter(this -> file_path, curr_token().pos, "SyntaxError", "Invalid if condition");
        skip_start_of_block();
        auto while_block = parse_block();
        skip_token("end");
        return std::make_unique<WhileStmt>(tok.pos, std::move(cond_expr), std::move(while_block));
    }

    // 解析函数定义（新语法：fn x() end）
    if (curr_tok.type == TokenType::Func) {
        DEBUG_OUTPUT("parsing function");
        auto tok = skip_token("fn");
        // 读取函数名
        const std::string func_name = skip_token().text;

        // 解析参数列表（()包裹，逻辑不变）
        std::vector<std::string> func_params;
        if (curr_token().type == TokenType::LParen) {
            skip_token("(");
            while (curr_token().type != TokenType::RParen) {
                func_params.push_back(skip_token().text);
                // 处理参数间的逗号
                if (curr_token().type == TokenType::Comma) {
                    skip_token(",");
                } else if (curr_token().type != TokenType::RParen) {
                    err::error_reporter(this -> file_path, curr_token().pos, "SyntaxError", "Mismatched function parameters");
                }
            }
            skip_token(")");  // 跳过右括号
        }

        // 解析函数体（无大括号，用end结尾）
        skip_start_of_block();  // 跳过参数后的换行
        auto func_body = parse_block();
        skip_token("end");

        // 生成函数定义语句节点
        return std::make_unique<AssignStmt>(tok.pos,  func_name, std::make_unique<FnDeclExpr>(
            tok.pos,
            func_name,
            std::move(func_params),
            std::move(func_body)
        ));
    }


    // 解析return语句
    if (curr_tok.type == TokenType::Return) {
        DEBUG_OUTPUT("parsing return");
        auto tok = skip_token("return");
        // return后可跟表达式（也可无，视为返回nil）
        std::unique_ptr<Expr> return_expr = parse_expression();
        skip_end_of_ln();
        return std::make_unique<ReturnStmt>(tok.pos, std::move(return_expr));
    }

    // 解析break语句
    if (curr_tok.type == TokenType::Break) {
        DEBUG_OUTPUT("parsing break");
        auto tok = skip_token("break");
        skip_end_of_ln();
        return std::make_unique<BreakStmt>(tok.pos);
    }

    // 解析continue语句
    if (curr_tok.type == TokenType::Next) {
        DEBUG_OUTPUT("parsing next");
        auto tok = skip_token("next");
        skip_end_of_ln();
        return std::make_unique<NextStmt>(tok.pos);
    }

    // 解析import语句
    if (curr_tok.type == TokenType::Import) {
        DEBUG_OUTPUT("parsing import");
        auto tok = skip_token("import");
        // 读取模块路径
        const std::string import_path = skip_token().text;

        skip_end_of_ln();
        return std::make_unique<ImportStmt>(tok.pos, import_path);
    }

    // 解析nonlocal语句
    if (curr_tok.type == TokenType::Nonlocal) {
        DEBUG_OUTPUT("parsing nonlocal");
        auto tok = skip_token("nonlocal");
        const std::string name = skip_token().text;
        skip_token("=");
        std::unique_ptr<Expr> expr = parse_expression();
        skip_end_of_ln();
        return std::make_unique<NonlocalAssignStmt>(tok.pos, name, std::move(expr));
    }

    // 解析global语句
    if (curr_tok.type == TokenType::Global) {
        DEBUG_OUTPUT("parsing global");
        auto tok = skip_token("global");
        const std::string name = skip_token().text;
        skip_token("=");
        std::unique_ptr<Expr> expr = parse_expression();
        skip_end_of_ln();
        return std::make_unique<GlobalAssignStmt>(tok.pos, name, std::move(expr));
    }

    // 解析object语句（适配end结尾）
    if (curr_tok.type == TokenType::Object) {
        DEBUG_OUTPUT("parsing object");
        auto tok = skip_token("object");
        skip_start_of_block();
        const std::string name = skip_token().text;
        auto object_block = parse_block();
        skip_token("end");
        return std::make_unique<ObjectStmt>(tok.pos, name, std::move(object_block));
    }
    
    // 解析throw语句
    if (curr_tok.type == TokenType::Throw) {
        DEBUG_OUTPUT("parsing throw");
        auto tok = skip_token("throw");
        std::unique_ptr<Expr> expr = parse_expression();
        skip_end_of_ln();
        return std::make_unique<ThrowStmt>(tok.pos, std::move(expr));
    }

    // 解析for语句
    if (curr_tok.type == TokenType::For) {
        DEBUG_OUTPUT("parsing for");
        auto tok = skip_token("for");
        const std::string name = skip_token().text;
        skip_token(":");
        std::unique_ptr<Expr> expr = parse_expression();

        skip_start_of_block();
        auto for_block = parse_block();
        skip_token("end");
        return std::make_unique<ForStmt>(tok.pos, name, std::move(expr), std::move(for_block));
    }
    
    // 解析try语句
    if (curr_tok.type == TokenType::Try) {
        DEBUG_OUTPUT("parsing try");
        auto tok = skip_token("try");
        
        skip_start_of_block();
        auto try_block = parse_block(TokenType::Catch);
        if (curr_token().type != TokenType::Catch)
            err::error_reporter(this -> file_path, curr_token().pos, "SyntaxError",
            "Found try block without catch block");

        std::vector<std::unique_ptr<CatchStmt>> catch_blocks;
        auto block_tok = curr_token();

        while (curr_token().type != TokenType::End) {
            DEBUG_OUTPUT("parsing catch");
            auto tok = skip_token("catch");
            const std::string name = skip_token().text;
            skip_token(":");
            std::unique_ptr<Expr> expr = parse_expression();

            skip_start_of_block();
            auto catch_block = parse_block(TokenType::Catch);
            catch_blocks.push_back(std::make_unique<CatchStmt>(tok.pos, std::move(expr), name, std::move(catch_block)));
        }

        skip_token("end");
        if (catch_blocks.empty()) {
            err::error_reporter(this -> file_path, curr_token().pos, "SyntaxError",
                "Nothing in catch block");
        }

        return std::make_unique<TryStmt>(tok.pos, std::move(try_block), std::move(catch_blocks));
    }

    // 解析赋值语句（x = expr;）
    if (curr_tok.type == TokenType::Identifier
        and curr_tok_idx_ + 1 < tokens_.size()
        and tokens_[curr_tok_idx_ + 1].type == TokenType::Assign
    ) {
        DEBUG_OUTPUT("parsing assign");
        const auto name_tok = skip_token();
        skip_token("=");
        auto expr = parse_expression();
        skip_end_of_ln();
        return std::make_unique<AssignStmt>(name_tok.pos, name_tok.text, std::move(expr));
    }


    // 解析表达式语句
    auto expr = parse_expression();
    if (expr != nullptr and curr_token().text == "=") {
        if (dynamic_cast<GetMemberExpr*>(expr.get())) {
            DEBUG_OUTPUT("parsing get member");
            skip_token("=");
            auto value = parse_expression();
            skip_end_of_ln();

            auto set_mem = std::make_unique<SetMemberStmt>(curr_token().pos, std::move(expr), std::move(value));
            return set_mem;
        }
        // 非成员访问表达式后不能跟 =
        err::error_reporter(this -> file_path, curr_token().pos, "SyntaxError",
            "Invalid assignment target: expected member access");
    }

    if (expr != nullptr) {
        skip_end_of_ln();
        return std::make_unique<ExprStmt>(curr_token().pos, std::move(expr));
    }

    // 跳过换行
    while (curr_token().type == TokenType::EndOfLine) {
        skip_token("\n");
    }

    return nullptr;  // 无有效语句，返回空
}

}
