#include "../../src/models/models.hpp"

namespace model {

// Nil.__eq__ 相等判断：仅当另一个对象也是Nil时返回true
Object* nil_eq(Object* self, const List* args) {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (nil_eq)");
    assert(args->val.size() == 1 && "function Nil.eq need 1 arg");
    
    auto self_nil = dynamic_cast<Nil*>(self);
    assert(self_nil != nullptr && "nil_eq must be called by Nil object");
    
    // Nil仅与自身相等
    auto another_nil = dynamic_cast<Nil*>(args->val[0]);
    return load_bool(another_nil != nullptr);
}

// Nil.__hash__
Object* nil_hash(Object* self, const List* args) {
    return create_int(0);
}

Object* nil_str(Object* self, const List* args) {
    return create_str("Nil");
}

}  // namespace model