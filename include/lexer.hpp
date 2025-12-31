/**
 * @file lexer.hpp
 * @brief 词法分析器（Lexer）核心定义
 * @author azhz1107cat
 * @date 2025-10-25
 */

#pragma once  
#include <string>
#include <vector>
#include <utility>
#include "error/error_reporter.hpp"
#include <cstddef>
#include <unordered_map>

namespace kiz {


// Token 类型与结构体
enum class TokenType {
    // 关键字
    Var, Func, If, Else, While, Return, Import, Break, Object,
    True, False, Nil, End, Next, Nonlocal, Global, Try, Catch, For,
    // 标识符
    Identifier,
    // 赋值运算符
    Assign,
    // 字面量
    Number, String,
    // 分隔符
    LParen, RParen, LBrace, RBrace, LBracket, RBracket,
    Comma, Dot, TripleDot, Semicolon,
    // 运算符
    ExclamationMark, Plus, Minus, Star, Slash, Backslash,
    Percent, Caret, Bang, Equal, NotEqual,
    Less, LessEqual, Greater, GreaterEqual, Pipe,
    FatArrow, ThinArrow, DoubleColon,
    Not, And, Or, Is, In,
    // 特殊标记
    EndOfFile, EndOfLine, Unknown
};

// Token定义
struct Token {
    TokenType type;
    std::string text;
    err::PositionInfo pos{};
    explicit Token( 
        TokenType tp,
        std::string t,
        size_t lno_start, size_t lno_end,
        size_t col_start, size_t col_end
    ) : type(tp) , text(std::move(t)) , pos{lno_start, lno_end, col_start, col_end} {}
    explicit Token(
        TokenType tp,
        std::string t,
        size_t lno, size_t col
    ) : type(tp) , text(std::move(t)) , pos{lno, lno, col, col} {}
};

// 词法分析器类
class Lexer {
    std::vector<Token> tokens;
    const std::string& file_path;
public:
    explicit Lexer(const std::string& file_path) : file_path(file_path) {}
    std::vector<Token> tokenize(const std::string& src, size_t lineno_start=1);
};

}  // namespace kiz