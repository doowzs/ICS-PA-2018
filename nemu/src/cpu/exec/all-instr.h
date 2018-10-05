#include "cpu/exec.h"

make_EHelper(mov);

make_EHelper(operand_size);

make_EHelper(inv);
make_EHelper(nemu_trap);

make_EHelper(call);
make_EHelper(push); //<-id_src
make_EHelper(pop);  //->id_dest

make_EHelper(add);
make_EHelper(sub);
make_EHelper(xor);
