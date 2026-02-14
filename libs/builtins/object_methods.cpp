#include "../../src/models/models.hpp"
#include "include/builtin_functions.hpp"

namespace model {


Object* range_call(Object* self, const List* args) {
    auto arg_vector = args->val;
    dep::BigInt start_int = 0;
    dep::BigInt step_int = 1;
    dep::BigInt end_int = 1;
    auto r_obj = new Object();
    r_obj->attrs_insert("__parent__", self);

    if (arg_vector.size() == 1) {
        end_int = cast_to_int(arg_vector[0])->val.to_unsigned_long_long();
    }
    else if (arg_vector.size() == 2) {
        start_int = cast_to_int(arg_vector[0])->val.to_unsigned_long_long();
        end_int = cast_to_int(arg_vector[1])->val.to_unsigned_long_long();
    }
    else if (arg_vector.size() == 3) {
        start_int = cast_to_int(arg_vector[0])->val.to_unsigned_long_long();
        step_int = cast_to_int(arg_vector[1])->val.to_unsigned_long_long();
        end_int = cast_to_int(arg_vector[2])->val.to_unsigned_long_long();

    } else kiz::Vm::assert_argc({1,2,3}, args);

    r_obj->attrs_insert("current", new Int(start_int));
    r_obj->attrs_insert("start", new Int(start_int));
    r_obj->attrs_insert("step", new Int(step_int));
    r_obj->attrs_insert("end", new Int(end_int));
    return r_obj;
}

Object* range_next(Object* self, const List* args) {
    Int* start_obj = cast_to_int(kiz::Vm::get_attr_current(self, "start"));
    Int* step_obj = cast_to_int(kiz::Vm::get_attr_current(self, "step"));
    Int* end_obj = cast_to_int(kiz::Vm::get_attr_current(self, "end"));
    Int* current_obj = cast_to_int(kiz::Vm::get_attr_current(self, "current"));

    dep::BigInt& start_int = start_obj->val;
    dep::BigInt& step_int = step_obj->val;
    dep::BigInt& end_int = end_obj->val;
    dep::BigInt& current = current_obj->val;

    bool is_end = false;
    if (step_int > 0) {
        is_end = current >= end_int;
    } else {
        return load_stop_iter_signal();
    }

    if (is_end) {
        return load_stop_iter_signal();
    }

    dep::BigInt old_val = current;
    current += step_int;

    // 返回迭代值
    return new Int(old_val);
}

Object* range_str(Object* self, const List* args) {
    dep::BigInt start_int = cast_to_int(kiz::Vm::get_attr_current(self, "start"))->val;
    dep::BigInt step_int = cast_to_int(kiz::Vm::get_attr_current(self, "step"))->val;
    dep::BigInt end_int = cast_to_int(kiz::Vm::get_attr_current(self, "end"))->val;
    dep::BigInt current = cast_to_int(kiz::Vm::get_attr_current(self, "current"))->val;

    return new String(std::format("Range(start={}, step={}, end={}, current={})", start_int.to_string(),
        step_int.to_string(), end_int.to_string(), current.to_string()));
}

}