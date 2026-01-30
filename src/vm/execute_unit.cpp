#include "vm.hpp"
#include "builtins/include/builtin_functions.hpp"
#include "op_code/opcode.hpp"

namespace kiz {
void execute_unit(const Instruction& instruction) {
    switch (instruction.opc) {
    case Opcode::OP_ADD:

    case Opcode::OP_SUB:

    case Opcode::OP_MUL:

    case Opcode::OP_DIV:

    case Opcode::OP_MOD:

    case Opcode::OP_POW:

    case Opcode::OP_NEG:


    case Opcode::OP_EQ:

    case Opcode::OP_GT:

    case Opcode::OP_LT:

    case Opcode::OP_GE:

    case Opcode::OP_LE:

    case Opcode::OP_NE:

    case Opcode::OP_AND:

    case Opcode::OP_NOT:

    case Opcode::OP_OR:

    case Opcode::OP_IS:

    case Opcode::OP_IN:



    case Opcode::MAKE_LIST:

    case Opcode::MAKE_DICT:


    case Opcode::CALL:

    case Opcode::RET:

    case Opcode::CALL_METHOD:

    case Opcode::GET_ATTR:

    case Opcode::SET_ATTR:

    case Opcode::GET_ITEM:

    case Opcode::SET_ITEM:

    case Opcode::LOAD_VAR:

    case Opcode::LOAD_CONST:

    case Opcode::SET_GLOBAL:

    case Opcode::SET_LOCAL:

    case Opcode::SET_NONLOCAL:


    case Opcode::ENTER_TRY:

    case Opcode::POP_TRY_FRAME:

    case Opcode::THROW:

    case Opcode::LOAD_ERROR:


    case Opcode::JUMP:

    case Opcode::JUMP_IF_FALSE:

    case Opcode::IS_INSTANCE:

    case Opcode::CREATE_OBJECT:

    case Opcode::IMPORT:

    case Opcode::STOP:

    default:                      assert(false && "execute_instruction: 未知 opcode");
    }

}
}
