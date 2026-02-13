#include "include/builtins_lib.hpp"

namespace builtins_lib {
model::Object* init_module(model::Object* self, const model::List* args) {
    auto mod = new model::Module("builtins_lib");

    mod->attrs_insert("print", model::create_nfunc(builtin::print, "print"));
    mod->attrs_insert("input", model::create_nfunc(builtin::input, "input"));
    mod->attrs_insert("ischild", model::create_nfunc(builtin::ischild, "ischild"));
    mod->attrs_insert("create", model::create_nfunc(builtin::create, "create"));
    mod->attrs_insert("now", model::create_nfunc(builtin::now, "now"));
    mod->attrs_insert("get_refc", model::create_nfunc(builtin::get_refc, "get_refc"));
    mod->attrs_insert("breakpoint", model::create_nfunc(builtin::breakpoint, "breakpoint"));
    mod->attrs_insert("cmd", model::create_nfunc(builtin::cmd, "cmd"));
    mod->attrs_insert("help", model::create_nfunc(builtin::help, "help"));
    mod->attrs_insert("delattr", model::create_nfunc(builtin::delattr, "delattr"));
    mod->attrs_insert("setattr", model::create_nfunc(builtin::setattr, "setattr"));
    mod->attrs_insert("getattr", model::create_nfunc(builtin::getattr, "getattr"));
    mod->attrs_insert("hasattr", model::create_nfunc(builtin::hasattr, "hasattr"));
    mod->attrs_insert("range", model::create_nfunc(builtin::range, "range"));
    mod->attrs_insert("type_of", model::create_nfunc(builtin::type_of_obj, "type_of"));
    mod->attrs_insert("debug_str", model::create_nfunc(builtin::debug_str, "debug_str"));
    mod->attrs_insert("attr", model::create_nfunc(builtin::attr, "attr"));
    mod->attrs_insert("sleep", model::create_nfunc(builtin::debug_str, "sleep"));
    
    mod->attrs_insert("Object", model::based_obj);
    mod->attrs_insert("Int", model::based_int);
    mod->attrs_insert("Bool", model::based_bool);
    mod->attrs_insert("Decimal", model::based_decimal);
    mod->attrs_insert("List", model::based_list);
    mod->attrs_insert("Dict", model::based_dict);
    mod->attrs_insert("Str", model::based_str);
    mod->attrs_insert("Func", model::based_function);
    mod->attrs_insert("NFunc", model::based_native_function);
    mod->attrs_insert("Error", model::based_error);
    mod->attrs_insert("Module", model::based_module);
    mod->attrs_insert("__CodeObject", model::based_code_object);
    mod->attrs_insert("__StopIterSignal__", model::stop_iter_signal);

    return mod;
}
}