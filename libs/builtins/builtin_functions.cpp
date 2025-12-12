#include "include/builtin_functions.hpp"

#include "models.hpp"

namespace builtin {

void register_builtin_functions() {
    print = [](model::Object* self, const model::List* args) -> model::Object* {
        std::string text;
        for (const auto* arg : args->val) {
            text += arg->to_string() + " ";
        }
        std::cout << text << std::endl;
        return new model::Nil();
    };

    input = [](model::Object* self, const model::List* args) -> model::Object* {
        const auto prompt_obj = get_one_arg(args);
        std::cout << prompt_obj->to_string();
        std::string result;
        std::getline(std::cin, result);
        return new model::String(result);
    };

    isinstance = [](model::Object* self, const model::List* args) -> model::Object* {
        if (!(args->val.size() == 2)) {
            assert(false && "函数参数不足两个");
        }

        const auto a = args->val[0];
        const auto b = args->val[1];
        return check_based_object(a, b);
    
    };

    help = [](model::Object* self, const model::List* args) -> model::Object* {

    };

    breakpointer = [](model::Object* self, const model::List* args) -> model::Object* {

    };

    range = [](model::Object* self, const model::List* args) -> model::Object* {

    };

    cmd = [](model::Object* self, const model::List* args) -> model::Object* {

    };

    now = [](model::Object* self, const model::List* args) -> model::Object* {

    };

    setattr = [](model::Object* self, const model::List* args) -> model::Object* {

    };

    getattr = [](model::Object* self, const model::List* args) -> model::Object* {

    };

    delattr = [](model::Object* self, const model::List* args) -> model::Object* {

    };

    getrefc = [](model::Object* self, const model::List* args) -> model::Object* {

    };

    copy = [](model::Object* self, const model::List* args) -> model::Object* {

    };

    create = [](model::Object* self, const model::List* args) -> model::Object* {

    };

    typeofobj = [](model::Object* self, const model::List* args) -> model::Object* {

    };
}

}
