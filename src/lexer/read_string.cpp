#include "lexer.hpp"

namespace kiz {
void Lexer::read_string() {
    dep::UTF8Char quote_char = src_[char_pos_];

    size_t start_char = char_pos_;
    size_t start_lno = lineno_;
    size_t start_col = col_;

    next(); // 跳过引号

    //bool closed = false;
    dep::UTF8String raw_str;

    // 消费字符串内容
    while (char_pos_ < src_.size()) {
        dep::UTF8Char c = src_[char_pos_];

        if (c == '\n') {
            break; // 普通字符串不允许跨行
        }

        if (c == quote_char) {
            //closed = true;
            next(); // 跳过闭合引号
            break;
        }

        if (c == '\\' && char_pos_ + 1 < src_.size()) {
            raw_str += c;
            next(); // 跳过转义符
            c = src_[char_pos_];
            raw_str += c;
            next(); // 消费转义后的字符
            continue;
        }

        raw_str += c;
        next();
    }

    std::string raw = raw_str.to_string();
    std::string content = handle_escape(raw);

    // if (!closed) {
    //     err::error_reporter(file_path_, {start_lno, lineno_, start_col, col_},
    //                       "SyntaxError", "Unclosed string literal");
    // }

    tokens_.emplace_back(TokenType::String, content, start_lno, lineno_, start_col, col_ - 1);
    curr_state_ = LexState::Start;
}


void Lexer::read_mstring() {
    size_t start_char = char_pos_;
    size_t start_lno = lineno_;
    size_t start_col = col_;

    next(); // 跳过M/m
    next(); // 跳过开头的双引号"

    bool unclosed = true;
    dep::UTF8String raw_str;

    // 消费字符串内容
    while (char_pos_ < src_.size()) {
        dep::UTF8Char c = src_[char_pos_];

        // 处理转义符
        if (c == '\\' && char_pos_ + 1 < src_.size()) {
            raw_str += c;
            next(); // 跳过转义符\
            c = src_[char_pos_];
            raw_str += c;
            next(); // 消费转义后的字符
            continue;
        }

        // 匹配非转义的闭合引号
        if (c == '"') {
            unclosed = false;
            next(); // 跳过闭合引号
            break;
        }

        raw_str += c;
        next(); // 消费字符
    }

    std::string raw = raw_str.to_string();
    std::string content = handle_escape(raw);

    if (unclosed) {
        err::error_reporter(file_path_, {start_lno, lineno_, start_col, col_},
                          "SyntaxError", R"(Unclosed multiline string literal (m"): missing closing '"')");
    }

    tokens_.emplace_back(TokenType::String, content, start_lno, lineno_, start_col, col_ - 1);
    curr_state_ = LexState::Start;
}

void Lexer::read_fstring() {
    size_t start_char = char_pos_;
    size_t start_lno = lineno_;
    size_t start_col = col_;

    dep::UTF8Char prefix = src_[char_pos_]; // 'f' 或 'F'
    next(); // 消费 'f' 或 'F'

    dep::UTF8Char quote_char = src_[char_pos_]; // 引号
    next(); // 消费引号

    // 生成 FStringStart token
    size_t fstring_start_char = start_char;
    size_t fstring_end_char = char_pos_; // 引号之后的位置
    emit_token(TokenType::FStringStart, fstring_start_char, fstring_end_char,
               start_lno, start_col, lineno_, col_ - 1);

    bool in_expr = false;
    size_t expr_start_char = 0;
    size_t expr_start_lno = 0;
    size_t expr_start_col = 0;
    int brace_depth = 0;

    while (char_pos_ < src_.size()) {
        dep::UTF8Char c = src_[char_pos_];

        // 检查是否结束引号
        if (c == quote_char) {
            // 生成 FStringEnd token
            size_t end_char = char_pos_;
            emit_token(TokenType::FStringEnd, end_char, end_char + 1,
                       lineno_, col_, lineno_, col_);
            next(); // 消费结束引号
            break;
        }

        // 处理转义字符（仅在表达式外部）
        if (c == '\\' && char_pos_ + 1 < src_.size() && !in_expr) {
            // 直接跳过转义序列，不生成token
            next(); // 跳过反斜杠
            if (char_pos_ < src_.size()) {
                next(); // 跳过转义字符
            }
            continue;
        }

        // 检查是否进入表达式
        if (c == '{' && !in_expr) {
            // 生成 InsertExprStart token
            size_t expr_start = char_pos_;
            emit_token(TokenType::InsertExprStart, expr_start, expr_start + 1,
                       lineno_, col_, lineno_, col_);
            next(); // 消费 '{'

            in_expr = true;
            brace_depth = 1;
            expr_start_char = char_pos_; // 表达式内容的开始
            expr_start_lno = lineno_;
            expr_start_col = col_;

            continue;
        }

        // 检查是否结束表达式
        if (c == '}' && in_expr) {
            // 首先提取表达式内容
            if (brace_depth == 1) {
                // 生成表达式标识符token
                if (expr_start_char < char_pos_) { // 表达式不为空
                    std::string expr_text;
                    for (size_t i = expr_start_char; i < char_pos_; i++) {
                        expr_text += src_[i].to_string();
                    }

                    // 生成 Identifier token
                    emit_token(TokenType::Identifier, expr_start_char, char_pos_,
                               expr_start_lno, expr_start_col, lineno_, col_ - 1);
                }

                // 生成 InsertExprEnd token
                size_t expr_end = char_pos_;
                emit_token(TokenType::InsertExprEnd, expr_end, expr_end + 1,
                           lineno_, col_, lineno_, col_);
                next(); // 消费 '}'

                in_expr = false;
                brace_depth = 0;
            } else {
                // 嵌套的 '}'，减少深度
                brace_depth--;
                next(); // 继续消费
            }
            continue;
        }

        // 处理表达式内的嵌套 '{'
        if (c == '{' && in_expr) {
            brace_depth++;
            next();
            continue;
        }

        // 普通字符串内容（不在表达式中）
        if (!in_expr) {
            size_t str_start = char_pos_;
            size_t str_start_lno = lineno_;
            size_t str_start_col = col_;

            // 收集直到下一个 '{' 或引号或转义符
            while (char_pos_ < src_.size()) {
                dep::UTF8Char ch = src_[char_pos_];
                if (ch == quote_char || ch == '{' || ch == '\\') {
                    break;
                }
                next();
            }

            // 如果有字符串内容，生成 String token
            if (char_pos_ > str_start) {
                std::string str_text;
                for (size_t i = str_start; i < char_pos_; i++) {
                    str_text += src_[i].to_string();
                }

                emit_token(TokenType::String, str_start, char_pos_,
                           str_start_lno, str_start_col, lineno_, col_ - 1);
            }
        } else {
            // 在表达式中，继续读取
            next();
        }
    }

    // 检查未闭合的表达式
    if (in_expr) {
        err::error_reporter(file_path_, {expr_start_lno, lineno_, expr_start_col, col_},
                          "SyntaxError", "Unclosed f-string expression");
    }

    curr_state_ = LexState::Start;
}

}
