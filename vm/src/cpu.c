#include <stdio.h>
#include <stdlib.h>

#include "mem.h"
#include "tty.h"

static ui64 PC = 0;
static ui64 registers[8];
static ui8 PWD = 0;

#define reg_of(ins) (registers[ins&0x07])

#define get_ins()      mem_read(PC, byte)
#define get_next_ins() mem_read(PC++, byte)

static inline void chpwd_cmp(byte args) {
    ui64 m, n;
    n = reg_of(args);
    m = registers[args >> 3];
    if (m<n)  PWD |= 0x4;
    if (m==n) PWD |= 0x2;
    if (m>n)  PWD |= 0x1;
}

static inline ui64 _read_imm(byte len) {
    ui64 imm = 0;
    while(len--)
        imm |= get_next_ins() << (len*8);
    return imm;
}

static inline void _binop(byte ins, byte arg) {
    if (ins & 0x20) {
        switch ((ins >> 3) & 0x3) {
            case 0: reg_of(arg>>3) %= reg_of(arg); break;
            case 1:
                    if ((ins & 0x7) == 1) {
                        reg_of(arg>>3) >>= reg_of(arg);
                    } else {
                        reg_of(arg>>3) <<= reg_of(arg);
                    }
                    break;
            case 2: reg_of(arg>>3) &= reg_of(arg); break;
            case 3: reg_of(arg>>3) |= reg_of(arg); break;
        }
    } else {
        if (ins & 0x1) { /* integers, with out checking sign */
            switch ((ins >> 3) & 0x3) {
                case 0: reg_of(arg>>3) += reg_of(arg); break;
                case 1: reg_of(arg>>3) -= reg_of(arg); break;
                case 2: reg_of(arg>>3) *= reg_of(arg); break;
                case 3: reg_of(arg>>3) /= reg_of(arg); break;
            }
        } else { /* floats */
            /* forget it */
            exit(233);
        }
    }
}

static inline void _mov(byte ins, byte arg) {
    ui64 imm = 0;
    size_t len = 4 << (arg & 0x07);
    if (arg & 0x04) { /* using Imm */
        len >>= 6; 
        imm = _read_imm(len);
        if (ins & 0x08) { /* write */
            mem_write_p(reg_of(arg >> 3), &imm, ui64);
        } else { /* read */
            reg_of(ins) = *(ui64*) mem_read_n(imm, len);
        }
    } else { /* using registers */
        if (ins & 0x08) { /* write */
            mem_write_n(reg_of(arg >> 3), &reg_of(arg), len);
        } else { /* read */
            reg_of(ins) = *(ui64*) mem_read_n(reg_of(arg >> 3), len);
        }
    }
}

static void mainloop() {
    ui64 imm;
    byte ins;
    while(1) {
loop:
        ins = get_next_ins();
        /* First router */
        switch(ins >> 6) {
            case 0:
                if(ins & 0x01) goto halt;
                if(ins & 0x08) goto jump;
                if(ins & 0x20) goto sop;
                break;
            case 1:
                goto binop;
            case 2:
                if(ins & 0x20) goto cmp;
                if(ins & 0x10) goto io;
                          else goto mov; 
            case 3:
                goto regop;
        } /* router end */
    }
/* sub routers */
regop:
    if (ins & 0x08) { /* reg-set */
        imm = _read_imm(get_next_ins() & 0x07);
        reg_of(ins) = imm;
    } else { /* reg-copy */
        reg_of(ins) = reg_of(get_next_ins());
    }
    goto loop;
sop:
    if ((ins & 0x18) == 1) {
        reg_of(ins) = !reg_of(ins);
    } else {
        reg_of(ins) = ~reg_of(ins);
    }
    goto loop;
binop:
    _binop(ins, get_next_ins());
    goto loop;
cmp:
    chpwd_cmp(get_next_ins());
    goto loop;
jump:
    if ((ins & 0x07) & PWD)
        PC = reg_of(get_next_ins());
    goto loop;
mov:
    
    _mov(ins, get_next_ins());
    goto loop;
io:
    if (ins & 0x08) { /* write to divices */
        tty_write_byte(reg_of(ins) & 0xFF); 
    } else { /* read from devices */
        reg_of(ins) = getchar();
    }
    goto loop;
#if(defined(TCVM_DEBUG_CPU))
halt:
    puts("halt.");
    exit(0);
error:
    printf("ERROR: Unknown instruction 0x%X\n", ins);
    exit(-2);
#else
halt:
    exit(0);
error:
    exit(-2);
#endif
}

int main() {
    byte code[] = {
        0xC8, 0x01, 'A',
        0xCA, 0x01, 0x01,
        0x41, 0x02,
        0xC9, 0x01, '\n',
        0x98,
        0x99,
        0x01 };
    mem_load_bytes(code, sizeof(code)/sizeof(byte));
    mainloop();
}
