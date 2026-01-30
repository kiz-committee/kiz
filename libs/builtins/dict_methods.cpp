#include "../../src/models/models.hpp"
#include "include/builtin_functions.hpp"

namespace model {

dep::BigInt hash_object(Object* key_obj) {
    // hash对象
    auto hash_method = kiz::Vm::get_attr(key_obj, "__hash__");
    kiz::Vm::call_function(hash_method, new List({}), key_obj);

    const auto result = kiz::Vm::fetch_one_from_stack_top();
    assert(result != nullptr);

    const auto result_int = dynamic_cast<Int*>(result);
    assert(result_int != nullptr);
    dep::BigInt key_hash_val = result_int->val;
    return key_hash_val;
}

// Dictionary.__add__：添加键值对 self + x（key: String，value: 任意Object），返回新Dictionary（不可变语义）
Object* dict_add(Object* self, const List* args) {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (dict_add)");
    assert(args->val.size() == 2 && "function Dictionary.add need 2 args: (key: String, value: Object)");
    
    auto self_dict = dynamic_cast<Dictionary*>(self);
    assert(self_dict != nullptr && "dict_add must be called by Dictionary object");
    
    auto another_dict = dynamic_cast<Dictionary*>(args->val[0]);
    assert(another_dict != nullptr);

    auto self_dict_to_vec = self_dict->val.to_vector();
    auto another_dict_to_vec = another_dict->val.to_vector();

    self_dict_to_vec.insert(
        self_dict_to_vec.end(),
        another_dict_to_vec.begin(),
        another_dict_to_vec.end()
    );
    
    auto new_dict = new Dictionary(dep::Dict(
        self_dict_to_vec
    ));
    
    return new_dict;
};

// Dictionary.contains：判断是否包含指定键（key: String），返回Bool
Object* dict_contains(Object* self, const List* args) {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (dict_contains)");
    assert(args->val.size() == 1 && "function Dictionary.contains need 1 arg: (key: String)");
    
    auto self_dict = dynamic_cast<Dictionary*>(self);
    assert(self_dict != nullptr && "dict_contains must be called by Dictionary object");
    
    // 键
    auto key_obj = args->val[0];

    auto hash_method = kiz::Vm::get_attr(key_obj, "__hash__");
    kiz::Vm::call_function(hash_method, new List({}), key_obj);

    const auto result = kiz::Vm::fetch_one_from_stack_top();
    assert(result != nullptr);

    const auto result_int = dynamic_cast<Int*>(result);
    assert(result_int != nullptr);
    dep::BigInt key_hash_val = result_int->val;

    auto found_pair_it = self_dict->val.find(
        key_hash_val
    );

    if (found_pair_it) {
        return new Bool(true);
    }
    return new Bool(false);
};

Object* dict_setitem(Object* self, const List* args) {
    assert(args->val.size() == 2);
    auto self_dict = dynamic_cast<Dictionary*>(self);
    auto key_obj = args->val[0];
    auto value_obj = args->val[1];
    dep::BigInt key_hash_val = hash_object(key_obj);

    self_dict->val.insert(
        key_hash_val,
        std::pair{key_obj, value_obj}
    );
    return new Nil();
}

Object* dict_getitem(Object* self, const List* args) {
    auto self_dict = dynamic_cast<Dictionary*>(self);
    auto key_obj = builtin::get_one_arg(args);

    dep::BigInt key_hash_val = hash_object(key_obj);

    auto found_pair_it = self_dict->val.find(key_hash_val);
    if (found_pair_it) {
        return found_pair_it->value.second;
    }

    throw NativeFuncError("KeyError",
            "Undefined key " + key_obj->debug_string() + " in Dictionary object " + self->debug_string()
    );
}

}  // namespace model