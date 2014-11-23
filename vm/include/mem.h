#include "common.h"

#if (!defined(TCVM_MEM_H))
#define TCVM_MEM_H

/* All the RAM is divided into  4KB blocks (1024 blocks on 64MB),
 * each block is divided into 64 byte cells.  */
#define MEMORY_SIZE 64 << 20 /* 64MB */
#define CELL_SIZE   64       /* 64B  */
#define BLOCK_SIZE  4096     /* 4KB  */
#define MEM_PAGES   (MEMORY_SIZE/BLOCK_SIZE)
#define MEM_CELLS   (BLOCK_SIZE/CELL_SIZE)

typedef struct {
    ui64 block_id;
    ui64 bitmap;
    void * mem;
} memblock;

void * _mem_read(ui32 addr, size_t size);
void _mem_write_p(ui32 addr, byte * datap, size_t size);

#define mem_read(addr, type) \
    (*((type*)_mem_read(addr, sizeof(type))))

#define mem_write_p(addr, datap, type) \
   _mem_write_p(addr, datap, sizeof(type))

#endif
