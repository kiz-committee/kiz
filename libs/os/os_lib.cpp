#include "include/os_lib.hpp"

namespace os_lib {

model::Object* init_module(model::Object* self, const model::List* args) {
    auto mod = new model::Module("io_lib");

    mod->attrs_insert("args", model::create_nfunc(get_args));
    mod->attrs_insert("env", model::create_nfunc(get_env));
    mod->attrs_insert("exit", model::create_nfunc(exit));
    mod->attrs_insert("cwd", model::create_nfunc(cwd));
    mod->attrs_insert("chdir", model::create_nfunc(chdir));
    mod->attrs_insert("mkdir", model::create_nfunc(mkdir));
    mod->attrs_insert("rmdir", model::create_nfunc(rmdir));
    mod->attrs_insert("remove", model::create_nfunc(remove));

    return mod;
}

model::Object* get_args(model::Object* self, const model::List* args) {

}

model::Object* get_env(model::Object* self, const model::List* args) {

}

model::Object* exit(model::Object* self, const model::List* args) {

}

model::Object* cwd(model::Object* self, const model::List* args) {

}

model::Object* chdir(model::Object* self, const model::List* args) {

}

model::Object* mkdir(model::Object* self, const model::List* args) {

}

model::Object* rmdir(model::Object* self, const model::List* args) {

}

model::Object* remove(model::Object* self, const model::List* args) {

}

}