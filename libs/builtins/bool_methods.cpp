#include "../../src/models/models.hpp"
#include "include/builtin_functions.hpp"

namespace model {

// Bool.__call__
Object* bool_call(Object* self, const List* args) {
    const auto a = builtin::get_one_arg(args);
    return new Bool(
        kiz::Vm::is_true(a)
    );
}

Object* bool_str(Object* self, const List* args) {
    const auto s = dynamic_cast<Bool*>(self);
    return create_str(s->val ? "True" : "False");
}


// Bool.__eq__ 布尔值相等判断：self == args[0]（仅支持Bool与Bool比较）
Object* bool_eq(Object* self, const List* args) {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (bool_eq)");
    assert(args->val.size() == 1 && "function Bool.eq need 1 arg");
    
    auto self_bool = dynamic_cast<Bool*>(self);
    auto another_bool = dynamic_cast<Bool*>(args->val[0]);
    assert(another_bool != nullptr && "Bool.eq only supports Bool type argument");
    
    return load_bool(self_bool->val == another_bool->val);
};

// Bool.__hash__
Object* bool_hash(Object* self, const List* args) {
    auto self_bool = dynamic_cast<Bool*>(self);
    if (self_bool->val == true) {
        return create_int(1);
    }
    return create_int(0);
}


}  // namespace model