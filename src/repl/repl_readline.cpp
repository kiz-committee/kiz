#include <iosfwd>
#include <string>
#include <windows.h>
#include <chrono>
#include <istream>
#include "kiz.hpp"
#include "repl.hpp"


#ifdef _WIN32
    #include <windows.h>
#elif __APPLE__
    #include <ApplicationServices/ApplicationServices.h>
#elif __linux__
    #include <X11/Xlib.h>
    #include <X11/Xutil.h>
#endif

bool ui::if_pressing_shift() {

#ifdef _WIN32
    // Windows 实现
    return ((GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0);

#elif __APPLE__
    // macOS 实现
    // 使用 CoreGraphics API
    CGEventRef event = CGEventCreate(NULL);
    CGEventFlags flags = CGEventGetFlags(event);
    CFRelease(event);

    // 检查 Shift 键
    return (flags & kCGEventFlagMaskShift) != 0;

#elif __linux__
    // Linux 实现（使用 X11）
    static Display* display = nullptr;
    static Window root_window;

    if (!display) {
        display = XOpenDisplay(nullptr);
        if (!display) return false;
        root_window = DefaultRootWindow(display);
    }

    // 获取键盘状态
    char keys_return[32];
    XQueryKeymap(display, keys_return);

    // 获取 Shift 键的键码
    KeyCode shift_l = XKeysymToKeycode(display, XK_Shift_L);
    KeyCode shift_r = XKeysymToKeycode(display, XK_Shift_R);

    // 检查是否按下
    bool shift_pressed = false;
    if (shift_l != 0 && (keys_return[shift_l >> 3] & (1 << (shift_l & 7))))
        shift_pressed = true;
    if (shift_r != 0 && (keys_return[shift_r >> 3] & (1 << (shift_r & 7))))
        shift_pressed = true;

    return shift_pressed;
#else
    return true;
#endif
}

/**
 * @brief 使REPL接收完整输入
 * @param is 输入流
 * @param os 输出流
 * @result 完整输入
 */
std::string ui::get_whole_input(std::istream *is, std::ostream *os) {
    if (!is) {
        throw KizStopRunningSignal("istream pointer is null");
    }

    std::string input;

    while (true) {
        char ch = is->get();
        // 检查Ctrl+Enter或Shift+Enter组合键来结束输入
        if (if_pressing_shift() && ch == '\n') {
            std::string result = input;
            DEBUG_OUTPUT("final returns input: " << result);
            return result;
        } if (ch == '\n') {
            os->put('.');
            os->put('.');
            os->put('.');
            os->put(' ');
            input += ch;
            DEBUG_OUTPUT("Add \\n to input: " << input);
        } else {
            input += ch;
        }
    }
}