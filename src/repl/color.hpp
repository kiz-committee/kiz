#pragma once
#include <string>
#define ie(str) (with_highlighting ? (str) : "")
class Color {
public:
    // 基础控制码：重置所有属性
    std::string RESET,

    // 文本样式控制
    BOLD,        // 加粗
    FAINT,       // 淡色
    ITALIC,      // 斜体（部分终端支持）
    UNDERLINE,   // 下划线
    BLINK,       // 闪烁（部分终端支持）
    REVERSE,     // 反色显示
    HIDDEN,      // 隐藏文本（部分终端支持）

    // 前景色（文字颜色）- 标准色
    BLACK,      // 黑色
    RED,        // 红色
    GREEN,      // 绿色
    YELLOW,     // 黄色
    BLUE,       // 蓝色
    MAGENTA,    // 品红色
    CYAN,       // 青色
    WHITE,      // 白色

    // 前景色 - 高亮色（亮色系）
    BRIGHT_BLACK,   // 亮黑色（灰色）
    BRIGHT_RED,     // 亮红色
    BRIGHT_GREEN,   // 亮绿色
    BRIGHT_YELLOW,  // 亮黄色
    BRIGHT_BLUE,    // 亮蓝色
    BRIGHT_MAGENTA, // 亮品红色
    BRIGHT_CYAN,    // 亮青色
    BRIGHT_WHITE,   // 亮白色

    // 背景色 - 标准色
    BG_BLACK,       // 背景黑色
    BG_RED,         // 背景红色
    BG_GREEN,       // 背景绿色
    BG_YELLOW,      // 背景黄色
    BG_BLUE,        // 背景蓝色
    BG_MAGENTA,     // 背景品红色
    BG_CYAN,        // 背景青色
    BG_WHITE,       // 背景白色

    // 背景色 - 高亮色（亮色系）
    BG_BRIGHT_BLACK,  // 背景亮黑色（灰色）
    BG_BRIGHT_RED,    // 背景亮红色
    BG_BRIGHT_GREEN,  // 背景亮绿色
    BG_BRIGHT_YELLOW, // 背景亮黄色
    BG_BRIGHT_BLUE,   // 背景亮蓝色
    BG_BRIGHT_MAGENTA,// 背景亮品红色
    BG_BRIGHT_CYAN,   // 背景亮青色
    BG_BRIGHT_WHITE;  // 背景亮白色

    // 构造函数
    explicit Color(bool with_highlighting = true) {
        RESET = ie("\033[0m");
        BOLD = ie("\033[1m");
        FAINT = ie("\033[2m");
        ITALIC = ie("\033[3m");
        UNDERLINE = ie("\033[4m");
        BLINK = ie("\033[5m");
        REVERSE = ie("\033[7m");
        HIDDEN = ie("\033[8m");

        BLACK = ie("\033[30m");
        RED = ie("\033[31m");
        GREEN = ie("\033[32m");
        YELLOW = ie("\033[33m");
        BLUE = ie("\033[34m");
        MAGENTA = ie("\033[35m");
        CYAN = ie("\033[36m");
        WHITE = ie("\033[37m");

        BRIGHT_BLACK = ie("\033[90m");
        BRIGHT_RED = ie("\033[91m");
        BRIGHT_GREEN = ie("\033[92m");
        BRIGHT_YELLOW = ie("\033[93m");
        BRIGHT_BLUE = ie("\033[94m");
        BRIGHT_MAGENTA = ie("\033[95m");
        BRIGHT_CYAN = ie("\033[96m");
        BRIGHT_WHITE = ie("\033[97m");

        BG_BLACK = ie("\033[40m");
        BG_RED = ie("\033[41m");
        BG_GREEN = ie("\033[42m");
        BG_YELLOW = ie("\033[43m");
        BG_BLUE = ie("\033[44m");
        BG_MAGENTA = ie("\033[45m");
        BG_CYAN = ie("\033[46m");
        BG_WHITE = ie("\033[47m");

        BG_BRIGHT_BLACK = ie("\033[100m");
        BG_BRIGHT_RED = ie("\033[101m");
        BG_BRIGHT_GREEN = ie("\033[102m");
        BG_BRIGHT_YELLOW = ie("\033[103m");
        BG_BRIGHT_BLUE = ie("\033[104m");
        BG_BRIGHT_MAGENTA = ie("\033[105m");
        BG_BRIGHT_CYAN = ie("\033[106m");
        BG_BRIGHT_WHITE = ie("\033[107m");
    }
};
#undef ie

// 全局Color实例，用于兼容旧代码
extern Color ColorInstance;

// 初始化全局Color实例的函数
extern void init_color_instance(bool with_highlighting);

