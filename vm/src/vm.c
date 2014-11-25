#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "mem.h"
#include "cpu.h"

static byte code[] = {
    0xCC, 0x01, 0x01, /* EX <- 1 */
    0xCD, 0x01, 0x0F, /* FX <- &(loop) */
    0xCE, 0x01, 0x1a, /* GX <- &(halt) */
    0xC8, 0x01, 0x1A, /* AX <- &'H' 26  */
    0xC9, 0x01, 0x28, /* BX <- &'\n' */
    0xA0, 0x01,       /* CMP AX, BX  */
    0x09, 0x06,       /* AX > BX -> goto halt */
    0x82, 0x00,       /* CX <- $(AX) */
    0x41, 0x04,       /* AX += EX */ 
    0x9A,             /* PRINT */
    0x0F, 0x05,       /* goto loop */
    0x01,
    'H', 'e', 'l', 'l', 'o', ',', ' ', 'w', 'o', 'r', 'l', 'd', '!', '\n'
};

int main(int argc, char *argv[]) {
    FILE * img = NULL;
    byte * bootstrap = NULL;
    size_t img_size = 0;
    if (argc == 2) {
        /* load bootstrap memory image */
        img = fopen(argv[1], "rb");
        fseek(img, 0, SEEK_END);
        img_size = ftell(img);
        bootstrap = (byte *) malloc(img_size);
        fseek(img, 0, SEEK_SET);
        fread(bootstrap, sizeof(byte), img_size, img);
        mem_load_bytes(bootstrap, img_size);
        free(bootstrap);
    } else {
        /* print "Hello World!\n" if not given any image  */
        mem_load_bytes(code, sizeof(code)/sizeof(byte));
    }
    mainloop();
    mem_free();
}
