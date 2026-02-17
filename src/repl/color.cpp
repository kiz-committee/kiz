#include "color.hpp"

// 全局Color实例
Color ColorInstance(true);

// 初始化全局Color实例的函数
void init_color_instance(bool with_highlighting) {
    // 重新初始化ColorInstance
    ColorInstance = Color(with_highlighting);
}
