#pragma once
#include "../../../src/models/models.hpp"
#include "builtin_functions.hpp"
#include "builtin_methods.hpp"

namespace builtins_lib {
model::Object* init_module(model::Object* self, const model::List* args);
}