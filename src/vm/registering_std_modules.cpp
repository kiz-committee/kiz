#include "../include/models.hpp"
#include "../libs/math/include/math_lib.hpp"

namespace model {

void registering_std_modules() {
    std_modules.insert("math", new CppFunction(
        math_lib::_init_module_
    ));
}

} // namespace model