#pragma once
#include <functional>

#include "../../../src/models/models.hpp"

namespace model {

// Int 类型原生函数
Object* int_add(Object* self, const List* args);
Object* int_sub(Object* self, const List* args);
Object* int_mul(Object* self, const List* args);
Object* int_div(Object* self, const List* args);
Object* int_pow(Object* self, const List* args);
Object* int_mod(Object* self, const List* args);
Object* int_neg(Object* self, const List* args);
Object* int_eq(Object* self, const List* args);
Object* int_lt(Object* self, const List* args);
Object* int_gt(Object* self, const List* args);
Object* int_bool(Object* self, const List* args);
Object* int_call(Object* self, const List* args);
Object* int_hash(Object* self, const List* args);

// Decimal类型原生函数
Object* decimal_add(Object* self, const List* args);
Object* decimal_sub(Object* self, const List* args);
Object* decimal_mul(Object* self, const List* args);
Object* decimal_div(Object* self, const List* args);
Object* decimal_pow(Object* self, const List* args);
Object* decimal_neg(Object* self, const List* args);
Object* decimal_eq(Object* self, const List* args);
Object* decimal_lt(Object* self, const List* args);
Object* decimal_gt(Object* self, const List* args);
Object* decimal_bool(Object* self, const List* args);
Object* decimal_call(Object* self, const List* args);
Object* decimal_safe_div(Object* self, const List* args);

// Rational 类型原生函数
Object* rational_add(Object* self, const List* args);
Object* rational_sub(Object* self, const List* args);
Object* rational_mul(Object* self, const List* args);
Object* rational_div(Object* self, const List* args);
//Object* rational_pow(Object* self, const List* args);
//Object* rational_mod(Object* self, const List* args);
Object* rational_eq(Object* self, const List* args);
Object* rational_lt(Object* self, const List* args);
Object* rational_gt(Object* self, const List* args);

// Nil 类型原生函数
Object* nil_eq(Object* self, const List* args);

// Bool 类型原生函数
Object* bool_eq(Object* self, const List* args);
Object* bool_call(Object* self, const List* args);

// String 类型原生函数
Object* str_eq(Object* self, const List* args);
Object* str_add(Object* self, const List* args);
Object* str_mul(Object* self, const List* args);
Object* str_contains(Object* self, const List* args);
Object* str_call(Object* self, const List* args);
Object* str_bool(Object* self, const List* args);
Object* str_hash(Object* self, const List* args);

// Dict 类型原生函数
Object* dict_eq(Object* self, const List* args);
Object* dict_add(Object* self, const List* args);
Object* dict_contains(Object* self, const List* args);;

// List 类型原生函数
Object* list_eq(Object* self, const List* args);
Object* list_add(Object* self, const List* args);
Object* list_mul(Object* self, const List* args);
Object* list_call(Object* self, const List* args);
Object* list_bool(Object* self, const List* args);
Object* list_next(Object* self, const List* args);
// 普通方法
Object* list_contains(Object* self, const List* args);
Object* list_append(Object* self, const List* args);
Object* list_foreach(Object* self, const List* args);
Object* list_reverse(Object* self, const List* args);
Object* list_extend(Object* self, const List* args);
Object* list_pop(Object* self, const List* args);
Object* list_insert(Object* self, const List* args);
Object* list_find(Object* self, const List* args);
Object* list_map(Object* self, const List* args);
Object* list_count(Object* self, const List* args);
Object* list_filter(Object* self, const List* args);



}
