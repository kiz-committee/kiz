#include "vm.hpp"
#include "../models/models.hpp"
#include "builtins/include/builtin_functions.hpp"
#include "builtins/include/builtin_methods.hpp"

namespace kiz {

void Vm::entry_builtins() {
    builtins.insert("print", new model::NativeFunction(builtin::print));
    builtins.insert("input", new model::NativeFunction(builtin::input));
    builtins.insert("ischild", new model::NativeFunction(builtin::ischild));
    builtins.insert("create", new model::NativeFunction(builtin::create));
    builtins.insert("now", new model::NativeFunction(builtin::now));
    builtins.insert("get_refc", new model::NativeFunction(builtin::get_refc));
    builtins.insert("breakpoint", new model::NativeFunction(builtin::breakpoint));
    builtins.insert("cmd", new model::NativeFunction(builtin::cmd));
    builtins.insert("help", new model::NativeFunction(builtin::help));
    builtins.insert("delattr", new model::NativeFunction(builtin::delattr));
    builtins.insert("setattr", new model::NativeFunction(builtin::setattr));
    builtins.insert("getattr", new model::NativeFunction(builtin::getattr));
    builtins.insert("hasattr", new model::NativeFunction(builtin::hasattr));
    builtins.insert("range", new model::NativeFunction(builtin::range));
    builtins.insert("type_of", new model::NativeFunction(builtin::type_of_obj));


    DEBUG_OUTPUT("registering builtin objects...");
    builtins.insert("Object", model::based_obj);

    model::based_bool->attrs.insert("__parent__", model::based_obj);
    model::based_int->attrs.insert("__parent__", model::based_obj);
    model::based_nil->attrs.insert("__parent__", model::based_obj);
    model::based_rational->attrs.insert("__parent__", model::based_obj);
    model::based_function->attrs.insert("__parent__", model::based_obj);
    model::based_dict->attrs.insert("__parent__", model::based_obj);
    model::based_list->attrs.insert("__parent__", model::based_obj);
    model::based_str->attrs.insert("__parent__", model::based_obj);

    DEBUG_OUTPUT("registering magic methods...");

    // Object 基类 __eq__
    model::based_obj->attrs.insert("__eq__", new model::NativeFunction([](const model::Object* self, const model::List* args) -> model::Object* {
        const auto other_obj = builtin::get_one_arg(args);
        return new model::Bool(self == other_obj);
    }));
    model::based_obj->attrs.insert("__str__", new model::NativeFunction([](const model::Object* self, const model::List* args) -> model::Object* {
        return new model::String(self->to_string());
    }));

    model::based_obj->attrs.insert("__getitem__", new model::NativeFunction([](const model::Object* self, const model::List* args) -> model::Object* {
        auto attr = builtin::get_one_arg(args);
        auto attr_str = dynamic_cast<model::String*>(attr);
        assert(attr_str != nullptr);
        return get_attr(self, attr_str->val);
    }));

    model::based_obj->attrs.insert("__setitem__", new model::NativeFunction([](model::Object* self, model::List* args) -> model::Object* {
        assert(args->val.size() == 2);
        auto attr = args->val[0];
        auto attr_str = dynamic_cast<model::String*>(attr);
        assert(attr_str != nullptr);
        self->attrs.insert(attr_str->val, args->val[1]);
        return self;
    }));

    // Bool 类型魔法方法
    model::based_bool->attrs.insert("__eq__", new model::NativeFunction(model::bool_eq));
    model::based_bool->attrs.insert("__call__", new model::NativeFunction(model::bool_call));

    // Nil 类型魔法方法
    model::based_nil->attrs.insert("__eq__", new model::NativeFunction(model::nil_eq));

    // Int 类型魔法方法
    model::based_int->attrs.insert("__add__", new model::NativeFunction(model::int_add));
    model::based_int->attrs.insert("__sub__", new model::NativeFunction(model::int_sub));
    model::based_int->attrs.insert("__mul__", new model::NativeFunction(model::int_mul));
    model::based_int->attrs.insert("__div__", new model::NativeFunction(model::int_div));
    model::based_int->attrs.insert("__mod__", new model::NativeFunction(model::int_mod));
    model::based_int->attrs.insert("__pow__", new model::NativeFunction(model::int_pow));
    model::based_int->attrs.insert("__neg__", new model::NativeFunction(model::int_neg));
    model::based_int->attrs.insert("__gt__", new model::NativeFunction(model::int_gt));
    model::based_int->attrs.insert("__lt__", new model::NativeFunction(model::int_lt));
    model::based_int->attrs.insert("__eq__", new model::NativeFunction(model::int_eq));
    model::based_int->attrs.insert("__call__", new model::NativeFunction(model::int_call));
    model::based_int->attrs.insert("__bool__", new model::NativeFunction(model::int_bool));
    model::based_int->attrs.insert("__hash__", new model::NativeFunction(model::int_hash));

    // Decimal类型魔术方法
    model::based_decimal->attrs.insert("__add__", new model::NativeFunction(model::decimal_add));
    model::based_decimal->attrs.insert("__sub__", new model::NativeFunction(model::decimal_sub));
    model::based_decimal->attrs.insert("__mul__", new model::NativeFunction(model::decimal_mul));
    model::based_decimal->attrs.insert("__div__", new model::NativeFunction(model::decimal_div));
    model::based_decimal->attrs.insert("__pow__", new model::NativeFunction(model::decimal_pow));
    model::based_decimal->attrs.insert("__neg__", new model::NativeFunction(model::decimal_neg));
    model::based_decimal->attrs.insert("__gt__", new model::NativeFunction(model::decimal_gt));
    model::based_decimal->attrs.insert("__lt__", new model::NativeFunction(model::decimal_lt));
    model::based_decimal->attrs.insert("__eq__", new model::NativeFunction(model::decimal_eq));
    model::based_decimal->attrs.insert("__call__", new model::NativeFunction(model::decimal_call));
    model::based_decimal->attrs.insert("__bool__", new model::NativeFunction(model::decimal_bool));
    model::based_decimal->attrs.insert("safe_div", new model::NativeFunction(model::decimal_safe_div));

    // Rational 类型魔法方法
    model::based_rational->attrs.insert("__add__", new model::NativeFunction(model::rational_add));
    model::based_rational->attrs.insert("__sub__", new model::NativeFunction(model::rational_sub));
    model::based_rational->attrs.insert("__mul__", new model::NativeFunction(model::rational_mul));
    model::based_rational->attrs.insert("__div__", new model::NativeFunction(model::rational_div));
    model::based_rational->attrs.insert("__gt__", new model::NativeFunction(model::rational_gt));
    model::based_rational->attrs.insert("__lt__", new model::NativeFunction(model::rational_lt));
    model::based_rational->attrs.insert("__eq__", new model::NativeFunction(model::rational_eq));

    // Dictionary 类型魔法方法
    model::based_dict->attrs.insert("__add__", new model::NativeFunction(model::dict_add));
    model::based_dict->attrs.insert("__contains__", new model::NativeFunction(model::dict_contains));

    // List 类型魔法方法
    model::based_list->attrs.insert("__add__", new model::NativeFunction(model::list_add));
    model::based_list->attrs.insert("__mul__", new model::NativeFunction(model::list_mul));
    model::based_list->attrs.insert("__eq__", new model::NativeFunction(model::list_eq));
    model::based_list->attrs.insert("__call__", new model::NativeFunction(model::list_call));
    model::based_list->attrs.insert("__bool__", new model::NativeFunction(model::list_bool));
    model::based_list->attrs.insert("__next__", new model::NativeFunction(model::list_next));
    model::based_list->attrs.insert("append", new model::NativeFunction(model::list_append));
    model::based_list->attrs.insert("contains", new model::NativeFunction(model::list_contains));
    model::based_list->attrs.insert("foreach", new model::NativeFunction(model::list_foreach));
    model::based_list->attrs.insert("reverse", new model::NativeFunction(model::list_reverse));
    model::based_list->attrs.insert("extend", new model::NativeFunction(model::list_extend));
    model::based_list->attrs.insert("pop", new model::NativeFunction(model::list_pop));
    model::based_list->attrs.insert("insert", new model::NativeFunction(model::list_insert));
    model::based_list->attrs.insert("find", new model::NativeFunction(model::list_find));
    model::based_list->attrs.insert("map", new model::NativeFunction(model::list_map));
    model::based_list->attrs.insert("count", new model::NativeFunction(model::list_count));
    model::based_list->attrs.insert("filter", new model::NativeFunction(model::list_filter));

    // String 类型魔法方法
    model::based_str->attrs.insert("__add__", new model::NativeFunction(model::str_add));
    model::based_str->attrs.insert("__mul__", new model::NativeFunction(model::str_mul));
    model::based_str->attrs.insert("__eq__", new model::NativeFunction(model::str_eq));
    model::based_str->attrs.insert("__call__", new model::NativeFunction(model::str_call));
    model::based_str->attrs.insert("__bool__", new model::NativeFunction(model::str_bool));
    model::based_str->attrs.insert("__hash__", new model::NativeFunction(model::str_hash));
    model::based_str->attrs.insert("contains", new model::NativeFunction(model::str_contains));

    model::based_error->attrs.insert("__call__", new model::NativeFunction([](model::Object* self, model::List* args) -> model::Object* {   assert( args->val.size() == 2);
        auto err_name = args->val[0];
        auto err_msg = args->val[1];

        auto err = new model::Error();
        err->attrs.insert("__name__", err_name);
        err->attrs.insert("__msg__", err_msg);
        return err;
    }));

    builtins.insert("Int", model::based_int);
    builtins.insert("Bool", model::based_bool);
    builtins.insert("__Rational", model::based_rational);
    builtins.insert("Decimal", model::based_decimal);
    builtins.insert("List", model::based_list);
    builtins.insert("Dict", model::based_dict);
    builtins.insert("Str", model::based_str);
    builtins.insert("Func", model::based_function);
    builtins.insert("NFunc", model::based_native_function);
    builtins.insert("__Nil", model::based_nil);
    builtins.insert("Error", model::based_error);
    builtins.insert("Module", model::based_module);
}
}
