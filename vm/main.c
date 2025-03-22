#include "chunk.h"
#include "debug.h"
#include "vm.h"

int main()
{
    VM vm;
    init_vm(&vm);

    Chunk chunk = {0};
    init_chunk(&chunk);

    // const 1.2
    int constant = add_constant(&chunk, 1.2);
    write_chunk(&chunk, OP_CONSTANT, 123);
    write_chunk(&chunk, constant, 123);
    
    // const 3.4
    constant = add_constant(&chunk, 3.4);
    write_chunk(&chunk, OP_CONSTANT, 123);
    write_chunk(&chunk, constant, 123);
    
    // op 1.2 + 3.4
    write_chunk(&chunk, OP_ADD, 123);
    
    // const 5.6
    constant = add_constant(&chunk, 5.6);
    write_chunk(&chunk, OP_CONSTANT, 123);
    write_chunk(&chunk, constant, 123);
    
    // op 4.6 / 5.6
    write_chunk(&chunk, OP_DIVIDE, 123);
    // op -(0.821429)
    write_chunk(&chunk, OP_NEGATE, 123);
    // return -0.821429
    write_chunk(&chunk, OP_RETURN, 123);

    disassemble_chunk(&chunk, "test chunk");

    interpret(&vm, &chunk);

    free_vm(&vm);
    free_chunk(&chunk);
    return 0;
}