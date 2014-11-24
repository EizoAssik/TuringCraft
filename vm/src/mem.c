#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <iso646.h>

#include "common.h"
#include "mem.h"

#if(defined(TCVM_DEBUG_MEM))
static ui64       mem_bitmaps [MEM_PAGES];
#endif
static memblock * mem_blocks  [MEM_PAGES];
static byte       zero_buffer [sizeof(ui64)];

static memblock * block_alloc() {
   memblock * bk = NULL;
   bk = (memblock *) calloc(1, sizeof(memblock));
   bk->mem = calloc(CELL_SIZE, MEM_CELLS);
   assert(bk);
   return bk;
}

static void block_free(memblock * block) {
    if (block && !(block->mem)) {
        free(block->mem);
        free(block);
    }
}

static void block_safe_free(memblock * block) {
    if (block && !(block->bitmap) && !(block->mem)) {
        free(block->mem);
        free(block);
    }
}

/* 64 MB memory need 26 bit address.
 * |<14-bits>+<6-bits>+<6-bits>|
 * |--block--+--cell--+--byte--|
 */

#define assert_valid_addr(addr) assert(addr<(1<<26));

static inline memblock * get_block(i32 addr) {
    return mem_blocks[addr >> 12];
}

static inline void * phy_addr(ui64 addr) {
    return get_block(addr)->mem + (addr & 0xFFF);
}

static inline ui32 _mem_get_block_addr(ui32 addr) {
    return addr >> 12;
}

static bool in_memory(i32 addr) {
    assert_valid_addr(addr);
    return get_block(addr) != NULL;
}

void * _mem_read(ui32 addr, size_t size) {
    assert_valid_addr(addr);
    if (!in_memory(addr)) /* if not in memory, then must be 0 */
        return (void *) zero_buffer;
    return phy_addr(addr);
}

void _mem_write_p(ui32 addr, byte * datap, size_t size) {
    assert_valid_addr(addr);
    ui32 block_addr = _mem_get_block_addr(addr);
    if (!mem_blocks[block_addr]) {
        mem_blocks[block_addr] = block_alloc();
    }
    byte * paddr = (byte *) phy_addr(addr);
    while(size--)
        *paddr++ = *datap++;
#if(defined(TCVM_DEBUG_MEM))
    mem_bitmaps[block_addr] |= (addr >> 6) & 0x3F;
#endif
}

#if(defined(TCVM_DEBUG_MEM))
int main() {
    char * t = "Hello, World";
    ui32 v = 10086;
    _mem_write_p(1<<20, 4, &v);
    printf("%ld\n", *(ui32*)phy_addr(1<<20));
    _mem_write_p(1222, 4, &v);
    printf("%ld\n", *(ui32*)phy_addr(1222));
    return 0;
}
#endif
