#include "parser.hpp"

#include <algorithm>
#include <cassert>
#include <memory>
#include <vector>

#include "../kiz.hpp"


namespace kiz {

std::unique_ptr<Expr> Parser::parse_expression() {
    DEBUG_OUTPUT("parse the expression...");
    return parse_and_or(); // 直接调用合并后的函数
}

// 处理 and/or（优先级相同，左结合）
std::unique_ptr<Expr> Parser::parse_and_or() {
    DEBUG_OUTPUT("parsing and/or expression...");
    auto node = parse_comparison();
    
    while (
        curr_token().type == TokenType::And
        or curr_token().type == TokenType::Or
        or curr_token().type == TokenType::Is
    ) {
        auto op_token = skip_token(curr_token().text);
        auto right = parse_comparison(); // 解析右侧比较表达式
        node = std::make_unique<BinaryExpr>(
            curr_token().pos,
            std::move(op_token.text),
            std::move(node),
            std::move(right)
        );
    }
    return node;
}

std::unique_ptr<Expr> Parser::parse_comparison() {
    DEBUG_OUTPUT("parsing comparison...");
    auto node = parse_add_sub();
    while (
        curr_token().type == TokenType::Equal
        or curr_token().type == TokenType::NotEqual
        or curr_token().type == TokenType::Greater
        or curr_token().type == TokenType::Less
        or curr_token().type == TokenType::GreaterEqual
        or curr_token().type == TokenType::LessEqual
    ) {
        auto tok = curr_token();
        auto op = skip_token().text;
        auto right = parse_add_sub();
        node = std::make_unique<BinaryExpr>(
            curr_token().pos,
            std::move(op),
            std::move(node),
            std::move(right)
        );
    }
    return node;
}

std::unique_ptr<Expr> Parser::parse_add_sub() {
    DEBUG_OUTPUT("parsing add/sub...");
    auto node = parse_mul_div_mod();
    while (
        curr_token().type == TokenType::Plus
        or curr_token().type == TokenType::Minus
    ) {
        auto tok = curr_token();
        auto op = skip_token().text;
        auto right = parse_mul_div_mod();
        node = std::make_unique<BinaryExpr>(curr_token().pos, std::move(op), std::move(node), std::move(right));
    }
    return node;
}

std::unique_ptr<Expr> Parser::parse_mul_div_mod() {
    DEBUG_OUTPUT("parsing mul/div/mod...");
    auto node = parse_power();
    while (
        curr_token().type == TokenType::Star
        or curr_token().type == TokenType::Slash
        or curr_token().type == TokenType::Percent
    ) {
        auto tok = curr_token();
        auto op = skip_token().text;
        auto right = parse_power();
        node = std::make_unique<BinaryExpr>(curr_token().pos, std::move(op), std::move(node), std::move(right));
    }
    return node;
}

std::unique_ptr<Expr> Parser::parse_power() {
    DEBUG_OUTPUT("parsing power...");
    auto node = parse_unary();
    if (curr_token().type == TokenType::Caret) {
        auto tok = curr_token();
        auto op = skip_token().text;
        auto right = parse_power();  // 右结合
        node = std::make_unique<BinaryExpr>(curr_token().pos, std::move(op), std::move(node), std::move(right));
    }
    return node;
}

std::unique_ptr<Expr> Parser::parse_unary() {
    DEBUG_OUTPUT("parsing unary...");
    if (curr_token().type == TokenType::Not) {
        auto op_token = skip_token(); // 跳过 not
        auto operand = parse_unary(); // 右结合
        return std::make_unique<UnaryExpr>(
            curr_token().pos,
            std::move(op_token.text),
            std::move(operand)
        );
    }
    if (curr_token().type == TokenType::Minus) {
        skip_token();
        auto operand = parse_unary();
        return std::make_unique<UnaryExpr>(curr_token().pos, "-", std::move(operand));
    }
    return parse_factor();
}

std::unique_ptr<Expr> Parser::parse_factor() {
    DEBUG_OUTPUT("parsing factor...");
    auto node = parse_primary();

    while (true) {
        if (curr_token().type == TokenType::Dot) {
            auto tok = curr_token();

            skip_token(".");
            auto child = std::make_unique<IdentifierExpr>(tok.pos, skip_token().text);
            node = std::make_unique<GetMemberExpr>(tok.pos, std::move(node),std::move(child));

        }
        else if (curr_token().type == TokenType::LBracket) {
            auto tok = curr_token();

            skip_token("[");
            auto param = parse_args(TokenType::RBracket);
            skip_token("]");
            node = std::make_unique<GetItemExpr>(tok.pos, std::move(node),std::move(param));
        }
        else if (curr_token().type == TokenType::LParen) {
            auto tok = curr_token();
            skip_token("(");
            auto param = parse_args(TokenType::RParen);
            skip_token(")");
            node = std::make_unique<CallExpr>(tok.pos, std::move(node),std::move(param));
        }
        else break;
    }
    return node;
}

std::unique_ptr<Expr> Parser::parse_primary() {
    DEBUG_OUTPUT("parsing primary...");
    const auto tok = skip_token();
    if (tok.type == TokenType::Number) {
        return std::make_unique<NumberExpr>(tok.pos, tok.text);
    }
    if (tok.type == TokenType::Decimal) {
        return std::make_unique<DecimalExpr>(tok.pos, tok.text);
    }
    if (tok.type == TokenType::String) {
        return std::make_unique<StringExpr>(tok.pos, tok.text);
    }
    if (tok.type == TokenType::Nil) {
        return std::make_unique<NilExpr>(tok.pos);
    }
    if (tok.type == TokenType::True) {
        return std::make_unique<BoolExpr>(tok.pos, true);
    }
    if (tok.type == TokenType::False) {
        return std::make_unique<BoolExpr>(tok.pos, false);
    }
    if (tok.type == TokenType::Identifier) {
        return std::make_unique<IdentifierExpr>(tok.pos, tok.text);
    }
    if (tok.type == TokenType::Func) {
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
                    assert(false && "Mismatched function parameters");
                }
            }
            skip_token(")");  // 跳过右括号
        }

        // 解析函数体（无大括号，用end结尾）
        skip_start_of_block();  // 跳过参数后的换行
        auto func_body = parse_block();
        skip_token("end");
        return std::make_unique<FnDeclExpr>(curr_token().pos, "<lambda>", std::move(func_params),std::move(func_body));
    }
    if (tok.type == TokenType::Pipe) {
        std::vector<std::string> params;
        while (curr_token().type != TokenType::Pipe) {
            params.emplace_back(skip_token().text);
            if (curr_token().type == TokenType::Comma) skip_token(",");
        }
        skip_token("|");
        auto expr = parse_expression();
        std::vector<std::unique_ptr<Stmt>> stmts;
        stmts.emplace_back(std::make_unique<ReturnStmt>(curr_token().pos, std::move(expr)));

        return std::make_unique<FnDeclExpr>(
            curr_token().pos,
            "lambda",
            std::move(params),
            std::make_unique<BlockStmt>(curr_token().pos, std::move(stmts))
        );
    }
    if (tok.type == TokenType::LBrace) {
        decltype(DictExpr::elements) init_vec{};
        while (curr_token().type != TokenType::RBrace) {
            auto key = parse_expression();
            skip_token(":");
            auto val = parse_expression();

            if (curr_token().type == TokenType::Comma) skip_token(",");
            else if (curr_token().type == TokenType::Semicolon) skip_token(";");
            else assert(false);

            init_vec.emplace_back(std::move(key), std::move(val));
        }
        skip_token("}");
        return std::make_unique<DictExpr>(curr_token().pos, std::move(init_vec));
    }
    if (tok.type == TokenType::LBracket) {
        auto param = parse_args(TokenType::RBracket);
        skip_token("]");
        return std::make_unique<ListExpr>(curr_token().pos, std::move(param));
    }
    if (tok.type == TokenType::LParen) {
        auto expr = parse_expression();
        skip_token(")");
        return expr;
    }
    return nullptr;
}

std::vector<std::unique_ptr<Expr>> Parser::parse_args(const TokenType endswith){
    std::vector<std::unique_ptr<Expr>> params;
    while (curr_token().type != endswith) {
        params.emplace_back(parse_expression());
        if (curr_token().type == TokenType::Comma) skip_token(",");
    }
    return params;
}

}
