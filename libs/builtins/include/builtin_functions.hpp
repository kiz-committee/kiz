#pragma once
#include <unordered_set>

#include "models.hpp"

namespace builtin {

inline model::Object* get_one_arg(const model::List* args) {
    if (!args->val.empty()) {
        return args->val[0];
    }
    assert(false && "函数参数不足一个");
}

inline model::Object* check_based_object_inner(
    model::Object* src_obj,
    model::Object* for_check_obj,
    std::unordered_set<model::Object*>& visited
) {
    if (src_obj == nullptr) return nullptr;
    // 闭环检测
    if (visited.contains(src_obj)) return new model::Bool(false);
    visited.insert(src_obj);

    // 查找__parent__属性
    const auto it = src_obj->attrs.find("__parent__");
    if (it == nullptr) {
        return new model::Bool(false);
    }
    // 找到目标返回true，否则递归检查父对象
    if (it->value == for_check_obj) return new model::Bool(true);
    return check_based_object_inner(it->value, for_check_obj, visited);
}

// 对外接口
inline model::Object* check_based_object(model::Object* src_obj, model::Object* for_check_obj) {
    std::unordered_set<model::Object*> visited; // 每次调用新建集合
    return check_based_object_inner(src_obj, for_check_obj, visited);
}

void register_builtin_functions();

using NativeFuncPtr = std::function<model::Object*(model::Object*, const model::List*)>;

inline NativeFuncPtr print;

inline NativeFuncPtr input;

inline NativeFuncPtr isinstance;

inline NativeFuncPtr help;

inline NativeFuncPtr breakpointer;

inline NativeFuncPtr range;

inline NativeFuncPtr cmd;

inline NativeFuncPtr now;

inline NativeFuncPtr setattr;

inline NativeFuncPtr getattr;

inline NativeFuncPtr delattr;

inline NativeFuncPtr getrefc;

inline NativeFuncPtr copy;

inline NativeFuncPtr create;

inline NativeFuncPtr typeofobj;

}