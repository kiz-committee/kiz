/**
 * @file lexer.cpp
 * @brief 词法分析器（FSM）核心实现 - 基于UTF-8 UTF8Char/UTF8String
 * @author azhz1107cat
 * @date 2025-10-25 + 2026-01-31 重构 + 修复UTF8Char使用
 */
#include "lexer.hpp"
#include <algorithm>
#include <cassert>

namespace kiz {

// 初始化关键字
void Lexer::init_keywords() {
    if (!keywords_.empty()) return;
    keywords_ = {
        {"if", TokenType::If},
        {"else", TokenType::Else},
        {"while", TokenType::While},
        {"for", TokenType::For},
        {"break", TokenType::Break},
        {"next", TokenType::Next},
        {"try", TokenType::Try},
        {"catch", TokenType::Catch},
        {"finally", TokenType::Finally},
        {"throw", TokenType::Throw},
        {"import", TokenType::Import},
        {"nonlocal", TokenType::Nonlocal},
        {"global", TokenType::Global},
        {"fn", TokenType::Func},
        {"object", TokenType::Object},
        {"return", TokenType::Return},
        {"end", TokenType::End},
        {"True", TokenType::True},
        {"False", TokenType::False},
        {"Nil", TokenType::Nil},
        {"and", TokenType::And},
        {"or", TokenType::Or},
        {"not", TokenType::Not},
        {"is", TokenType::Is},
        {"in", TokenType::In}
    };
}

// 读取当前码点并移动位置
dep::UTF8Char Lexer::next() {
    if (cp_pos_ >= total_cp_) return dep::UTF8Char('\0');

    dep::UTF8Char ch = src_[cp_pos_];
    cp_pos_++;

    // 更新行号/列号：按Unicode码点计数
    if (ch == CHAR_NEWLINE) {
        lineno_++;
        col_ = 1;
    } else if (ch != CHAR_RETURN) { // 跳过回车符，避免重复行号
        col_++;
    }

    return ch;
}

// 处理字符串转义
std::string Lexer::handle_escape(const std::string& raw) {
    std::string res;
    res.reserve(raw.size());

    for (size_t i = 0; i < raw.size(); ++i) {
        if (raw[i] == '\\' && i + 1 < raw.size()) {
            switch (raw[++i]) {
            case 'n': res += '\n'; break;
            case 't': res += '\t'; break;
            case 'r': res += '\r'; break;
            case '\\': res += '\\'; break;
            case '"': res += '"'; break;
            case '\'': res += '\''; break;
            default:
                res += '\\';  // 保留转义符
                res += raw[i]; // 和未知字符
                break;
            }
        } else {
            res += raw[i];
        }
    }
    return res;
}

// 生成Token：按码点范围取原始字节
void Lexer::emit_token(TokenType type, size_t start_cp, size_t end_cp,
                       size_t start_lno, size_t start_col,
                       size_t end_lno, size_t end_col) {
    dep::UTF8String substr;
    for (size_t i = start_cp; i < end_cp && i < src_.size(); ++i) {
        substr += src_[i];
    }

    std::string text = substr.to_string();
    tokens_.emplace_back(type, text, start_lno, end_lno, start_col, end_col);
}

// 快速生成单码点Token
void Lexer::emit_single_cp_token(TokenType type, size_t cp_index) {
    // 注意：cp_index应该是消费前的索引
    size_t current_cp = cp_index;
    emit_token(type, current_cp, current_cp + 1,
               lineno_, col_ - 1, lineno_, col_ - 1);
}

// 核心：有限自动状态机 词法分析
std::vector<Token> Lexer::tokenize(const std::string& src, size_t lineno_start) {
    // 初始化状态
    src_ = dep::UTF8String(src);
    tokens_.clear();
    curr_state_ = LexState::Start;
    cp_pos_ = 0;
    lineno_ = lineno_start;
    col_ = 1;
    total_cp_ = src_.size();

    while (cp_pos_ < total_cp_) {
        dep::UTF8Char current_char = src_[cp_pos_];

        switch (curr_state_) {
        // ======================================
        // 初始状态：核心分支
        // ======================================
        case LexState::Start: {
            if (is_space(current_char)) {
                // 空白符
                if (is_newline(current_char)) {
                    bool has_bs = !tokens_.empty() && tokens_.back().type == TokenType::Backslash;
                    if (!has_bs) {
                        emit_single_cp_token(TokenType::EndOfLine, cp_pos_);
                    } else {
                        tokens_.pop_back(); // 移除续行符
                    }
                }
                next(); // 消费空白符
            }
            else if ((current_char == CHAR_M || current_char == CHAR_m) && cp_pos_ + 1 < total_cp_ &&
                     peek() == CHAR_QUOTE) {
                // 跨行字符串 M"/m"
                curr_state_ = LexState::MultilineString;
                     }
            else if (is_alpha_under(current_char)) {
                curr_state_ = LexState::Identifier;
            }
            else if (is_digit(current_char) || (current_char == CHAR_DOT && cp_pos_ + 1 < total_cp_ &&
                     is_digit(peek()))) {
                curr_state_ = LexState::Number;
                     }
            else if (current_char == CHAR_HASH) {
                curr_state_ = LexState::SingleComment;
            }
            else if (current_char == CHAR_SLASH && cp_pos_ + 1 < total_cp_ &&
                     peek() == CHAR_STAR) {
                curr_state_ = LexState::BlockComment;
                     }
            else if (current_char == CHAR_QUOTE || current_char == CHAR_SQUOTE) {
                curr_state_ = LexState::String;
            }
            else if (current_char == CHAR_EQUAL || current_char == CHAR_EXCLAM ||
                     current_char == CHAR_LESS || current_char == CHAR_GREATER ||
                     current_char == CHAR_MINUS || current_char == CHAR_COLON) {
                curr_state_ = LexState::Operator;
                     }
            else {
                // 单字符Token处理
                size_t start_pos = cp_pos_;
                if (current_char == CHAR_LPAREN) {
                    next();
                    emit_token(TokenType::LParen, start_pos, cp_pos_,
                              lineno_, col_ - 1, lineno_, col_ - 1);
                }
                else if (current_char == CHAR_RPAREN) {
                    next();
                    emit_token(TokenType::RParen, start_pos, cp_pos_,
                              lineno_, col_ - 1, lineno_, col_ - 1);
                }
                else if (current_char == CHAR_LBRACE) {
                    next();
                    emit_token(TokenType::LBrace, start_pos, cp_pos_,
                              lineno_, col_ - 1, lineno_, col_ - 1);
                }
                else if (current_char == CHAR_RBRACE) {
                    next();
                    emit_token(TokenType::RBrace, start_pos, cp_pos_,
                              lineno_, col_ - 1, lineno_, col_ - 1);
                }
                else if (current_char == CHAR_LBRACKET) {
                    next();
                    emit_token(TokenType::LBracket, start_pos, cp_pos_,
                              lineno_, col_ - 1, lineno_, col_ - 1);
                }
                else if (current_char == CHAR_RBRACKET) {
                    next();
                    emit_token(TokenType::RBracket, start_pos, cp_pos_,
                              lineno_, col_ - 1, lineno_, col_ - 1);
                }
                else if (current_char == CHAR_COMMA) {
                    next();
                    emit_token(TokenType::Comma, start_pos, cp_pos_,
                              lineno_, col_ - 1, lineno_, col_ - 1);
                }
                else if (current_char == CHAR_SEMICOLON) {
                    next();
                    emit_token(TokenType::Semicolon, start_pos, cp_pos_,
                              lineno_, col_ - 1, lineno_, col_ - 1);
                }
                else if (current_char == CHAR_PLUS) {
                    next();
                    emit_token(TokenType::Plus, start_pos, cp_pos_,
                              lineno_, col_ - 1, lineno_, col_ - 1);
                }
                else if (current_char == CHAR_STAR) {
                    next();
                    emit_token(TokenType::Star, start_pos, cp_pos_,
                              lineno_, col_ - 1, lineno_, col_ - 1);
                }
                else if (current_char == CHAR_BACKSLASH) {
                    next();
                    emit_token(TokenType::Backslash, start_pos, cp_pos_,
                              lineno_, col_ - 1, lineno_, col_ - 1);
                }
                else if (current_char == CHAR_PERCENT) {
                    next();
                    emit_token(TokenType::Percent, start_pos, cp_pos_,
                              lineno_, col_ - 1, lineno_, col_ - 1);
                }
                else if (current_char == CHAR_CARET) {
                    next();
                    emit_token(TokenType::Caret, start_pos, cp_pos_,
                              lineno_, col_ - 1, lineno_, col_ - 1);
                }
                else if (current_char == CHAR_PIPE) {
                    next();
                    emit_token(TokenType::Pipe, start_pos, cp_pos_,
                              lineno_, col_ - 1, lineno_, col_ - 1);
                }
                else if (current_char == CHAR_SLASH) {
                    next();
                    emit_token(TokenType::Slash, start_pos, cp_pos_,
                              lineno_, col_ - 1, lineno_, col_ - 1);
                }
                else if (current_char == CHAR_DOT) {
                    next(); // 消费第一个点
                    if (cp_pos_ < total_cp_ && src_[cp_pos_] == CHAR_DOT &&
                        cp_pos_ + 1 < total_cp_ && src_[cp_pos_ + 1] == CHAR_DOT) {
                        next(); // 消费第二个点
                        next(); // 消费第三个点
                        emit_token(TokenType::TripleDot, start_pos, cp_pos_,
                                  lineno_, col_ - 3, lineno_, col_);
                    } else {
                        emit_token(TokenType::Dot, start_pos, cp_pos_,
                                      lineno_, col_ - 1, lineno_, col_ - 1);
                    }
                }
                else {
                    // 未知字符：错误报告
                    err::error_reporter(file_path_, {lineno_, lineno_, col_, col_},
                                      "SyntaxError", "Unknown character");
                    next();
                    emit_token(TokenType::Unknown, start_pos, cp_pos_,
                              lineno_, col_ - 1, lineno_, col_ - 1);
                }
            }
            break;
        }

        // ======================================
        // 跨行字符串状态 M"/m"
        // ======================================
        case LexState::MultilineString: {
            size_t start_cp = cp_pos_;
            size_t start_lno = lineno_;
            size_t start_col = col_;

            next(); // 跳过M/m
            next(); // 跳过开头的双引号"

            bool unclosed = true;
            dep::UTF8String raw_str;

            // 消费字符串内容
            while (cp_pos_ < total_cp_) {
                dep::UTF8Char c = src_[cp_pos_];

                // 处理转义符
                if (c == CHAR_BACKSLASH && cp_pos_ + 1 < total_cp_) {
                    raw_str += c;
                    next(); // 跳过转义符\
                    c = src_[cp_pos_];
                    raw_str += c;
                    next(); // 消费转义后的字符
                    continue;
                }

                // 匹配非转义的闭合引号
                if (c == CHAR_QUOTE) {
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
            break;
        }

        // ======================================
        // 标识符/关键字状态
        // ======================================
        case LexState::Identifier: {
            size_t start_cp = cp_pos_;
            size_t start_lno = lineno_;
            size_t start_col = col_;

            dep::UTF8String ident_str;

            // 消费第一个字符（已经在当前状态）
            ident_str += src_[cp_pos_];
            next();

            // 消费标识符后续字符
            while (cp_pos_ < total_cp_ && is_ident_continue(src_[cp_pos_])) {
                ident_str += src_[cp_pos_];
                next();
            }

            std::string ident = ident_str.to_string();
            TokenType type = keywords_.contains(ident) ? keywords_[ident] : TokenType::Identifier;

            emit_token(type, start_cp, cp_pos_, start_lno, start_col, lineno_, col_ - 1);
            curr_state_ = LexState::Start;
            break;
        }

        // ======================================
        // 数字状态：整数/小数/科学计数法
        // ======================================
        case LexState::Number: {
            size_t start_cp = cp_pos_;
            size_t start_lno = lineno_;
            size_t start_col = col_;
            bool has_dot = false;
            bool has_sci = false;

            dep::UTF8String num_str;

            // 消费第一个数字或点
            num_str += src_[cp_pos_];
            if (src_[cp_pos_] == CHAR_DOT) {
                has_dot = true;
            }
            next();

            // 消费数字
            while (cp_pos_ < total_cp_) {
                dep::UTF8Char c = src_[cp_pos_];

                if (is_digit(c)) {
                    num_str += c;
                    next();
                }
                else if (c == CHAR_DOT && !has_dot && !has_sci) {
                    // 检查下一个字符是否为数字
                    if (cp_pos_ + 1 < total_cp_ && is_digit(peek())) {
                        has_dot = true;
                        num_str += c;
                        next();
                    } else {
                        break;
                    }
                }
                else if ((c == CHAR_e || c == CHAR_E) && !has_sci) {
                    // 科学计数法
                    has_sci = true;
                    num_str += c;
                    next();

                    // 处理科学计数法的正负号
                    if (cp_pos_ < total_cp_) {
                        dep::UTF8Char next_c = src_[cp_pos_];
                        if (next_c == CHAR_PLUS || next_c == CHAR_MINUS) {
                            num_str += next_c;
                            next();
                        }
                    }

                    // 科学计数法后必须跟数字
                    if (cp_pos_ >= total_cp_ || !is_digit(src_[cp_pos_])) {
                        break;
                    }
                }
                else {
                    break;
                }
            }

            // 判定类型
            TokenType type = (has_sci || has_dot) ? TokenType::Decimal : TokenType::Number;
            emit_token(type, start_cp, cp_pos_, start_lno, start_col, lineno_, col_ - 1);
            curr_state_ = LexState::Start;
            break;
        }

        // ======================================
        // 运算符状态：处理双字符运算符/
        // ======================================
        case LexState::Operator: {
            size_t start_cp = cp_pos_;
            size_t start_lno = lineno_;
            size_t start_col = col_;

            dep::UTF8Char c1 = src_[cp_pos_];
            next(); // 消费第一个字符

            TokenType type = TokenType::Unknown;
            size_t token_end = cp_pos_;

            // 检查第二个字符
            if (cp_pos_ < total_cp_) {
                dep::UTF8Char c2 = src_[cp_pos_];

                // 匹配双字符运算符
                if (c1 == CHAR_EQUAL && c2 == CHAR_GREATER) {
                    type = TokenType::FatArrow;
                    next(); // 消费第二个字符
                    token_end = cp_pos_;
                }
                else if (c1 == CHAR_MINUS && c2 == CHAR_GREATER) {
                    type = TokenType::ThinArrow;
                    next(); // 消费第二个字符
                    token_end = cp_pos_;
                }
                else if (c1 == CHAR_EQUAL && c2 == CHAR_EQUAL) {
                    type = TokenType::Equal;
                    next(); // 消费第二个字符
                    token_end = cp_pos_;
                }
                else if (c1 == CHAR_EXCLAM && c2 == CHAR_EQUAL) {
                    type = TokenType::NotEqual;
                    next(); // 消费第二个字符
                    token_end = cp_pos_;
                }
                else if (c1 == CHAR_LESS && c2 == CHAR_EQUAL) {
                    type = TokenType::LessEqual;
                    next(); // 消费第二个字符
                    token_end = cp_pos_;
                }
                else if (c1 == CHAR_GREATER && c2 == CHAR_EQUAL) {
                    type = TokenType::GreaterEqual;
                    next(); // 消费第二个字符
                    token_end = cp_pos_;
                }
                else if (c1 == CHAR_COLON && c2 == CHAR_EQUAL) {
                    type = TokenType::Assign;
                    next(); // 消费第二个字符
                    token_end = cp_pos_;
                }
            }

            // 单字符运算符
            if (type == TokenType::Unknown) {
                token_end = cp_pos_; // 只消费了一个字符
                if (c1 == CHAR_EQUAL) type = TokenType::Assign;
                else if (c1 == CHAR_EXCLAM) type = TokenType::ExclamationMark;
                else if (c1 == CHAR_LESS) type = TokenType::Less;
                else if (c1 == CHAR_GREATER) type = TokenType::Greater;
                else if (c1 == CHAR_COLON) type = TokenType::Colon;
                else if (c1 == CHAR_MINUS) type = TokenType::Minus;
            }

            emit_token(type, start_cp, token_end, start_lno, start_col, lineno_, col_ - 1);
            curr_state_ = LexState::Start;
            break;
        }

        // ======================================
        // 普通字符串状态：""/''
        // ======================================
        case LexState::String: {
            dep::UTF8Char quote_char = src_[cp_pos_];

            size_t start_cp = cp_pos_;
            size_t start_lno = lineno_;
            size_t start_col = col_;

            next(); // 跳过引号

            //bool closed = false;
            dep::UTF8String raw_str;

            // 消费字符串内容
            while (cp_pos_ < total_cp_) {
                dep::UTF8Char c = src_[cp_pos_];

                if (is_newline(c)) {
                    break; // 普通字符串不允许跨行
                }

                if (c == quote_char) {
                    //closed = true;
                    next(); // 跳过闭合引号
                    break;
                }

                if (c == CHAR_BACKSLASH && cp_pos_ + 1 < total_cp_) {
                    raw_str += c;
                    next(); // 跳过转义符
                    c = src_[cp_pos_];
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
            break;
        }

        // ======================================
        // 单行注释状态：# 至行尾
        // ======================================
        case LexState::SingleComment: {
            next(); // 跳过#

            // 消费至行尾
            while (cp_pos_ < total_cp_ && !is_newline(src_[cp_pos_])) {
                next();
            }

            curr_state_ = LexState::Start;
            break;
        }

        // ======================================
        // 块注释状态：/* */ 支持跨行
        // ======================================
        case LexState::BlockComment: {
            next(); // 跳过/
            next(); // 跳过*

            // 消费至*/
            while (cp_pos_ < total_cp_) {
                if (cp_pos_ + 1 < total_cp_ &&
                    src_[cp_pos_] == CHAR_STAR &&
                    src_[cp_pos_ + 1] == CHAR_SLASH) {
                    next(); // 跳过*
                    next(); // 跳过/
                    break;
                    }
                next();
            }

            curr_state_ = LexState::Start;
            break;
        }
        }
    }

    // 生成EOF Token
    tokens_.emplace_back(TokenType::EndOfFile, "", lineno_, col_);
    return tokens_;
}
}