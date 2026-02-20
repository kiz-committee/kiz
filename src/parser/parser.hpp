/**
 * @file parser.hpp
 * @brief 语法分析器（Parser）核心定义
 * @author azhz1107cat
 * @date 2025-10-25
 */

#pragma once
#include "ast.hpp"
#include "../lexer/lexer.hpp"

#include <memory>
#include <string>

namespace kiz {

class Parser {
    std::vector<Token> tokens_;
    size_t curr_tok_idx_ = 0;
    const std::string& file_path;
public:
    explicit Parser(const std::string& file_path) : file_path(file_path) {}
    ~Parser() = default;

    Token skip_token(const std::string& want_skip = "");
    Token skip_token_allow_space(const std::string& want_skip = "");
    void skip_end_of_ln();
    void skip_start_of_block();
    [[nodiscard]] Token curr_token() const;

    std::unique_ptr<BlockStmt> parse(const std::vector<Token>& tokens);

private:
    // parse stmt
    std::unique_ptr<Stmt> parse_stmt();
    std::unique_ptr<BlockStmt> parse_block(TokenType endswith = TokenType::End);
    std::unique_ptr<IfStmt> parse_if();

    // parse expr
    std::unique_ptr<Expr> parse_expression();
    std::unique_ptr<Expr> parse_and_or();
    std::unique_ptr<Expr> parse_comparison();
    std::unique_ptr<Expr> parse_add_sub();
    std::unique_ptr<Expr> parse_mul_div_mod();
    std::unique_ptr<Expr> parse_power();
    std::unique_ptr<Expr> parse_unary();
    std::unique_ptr<Expr> parse_factor();

    // parse factor
    std::unique_ptr<Expr> parse_primary();
    std::vector<std::unique_ptr<Expr>> parse_args(TokenType endswith);
};

} // namespace kiz