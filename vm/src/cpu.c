#include <stdio.h>
#include <stdlib.h>

#include "mem.h"

static ui64 PC = 0;
static ui64 registers[8];
static ui8 PWD = 0;

#define reg_of(ins) (registers[ins&0x07])

#define get_ins()      mem_read(PC, byte)
#define get_next_ins() mem_read(PC++, byte)

static void chpwd_cmp(byte args) {
    ui64 m, n;
    n = reg_of(args);
    m = registers[args >> 3];
    if (m<n)  PWD |= 0x4;
    if (m==n) PWD |= 0x2;
    if (m>n)  PWD |= 0x1;
}

static void _binop(byte ins, byte args) {
    ui64 m, n;
    n = reg_of(args);
    m = registers[args >> 3];
    if (ins & 0x20) {
        switch ((ins >> 3) & 0x3) {
            case 0: reg_of(m) %= reg_of(n); break;
            case 1:
                    if ((ins & 0x7) == 1) {
                        reg_of(m) >>= reg_of(n);
                    } else {
                        reg_of(m) <<= reg_of(n);
                    }
                    break;
            case 2: reg_of(m) &= reg_of(n); break;
            case 3: reg_of(m) |= reg_of(n); break;
        }
    } else {
        if (ins & 0x1) { /* integers, with out checking sign */
            switch ((ins >> 3) & 0x3) {
                case 0: reg_of(m) += reg_of(n); break;
                case 1: reg_of(m) -= reg_of(n); break;
                case 2: reg_of(m) *= reg_of(n); break;
                case 3: reg_of(m) /= reg_of(n); break;
            }
        } else { /* floats */
            /* forget it */
            exit(233);
        }
    }
}

static void mainloop() {
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
                if(ins & 0x10) goto mov;
                          else goto io; 
            default:
                goto error;
        } /* router end */
    }
/* sub routers */
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
    if ((ins & 0x7) & PWD)
        PC = reg_of(get_next_ins());
    goto loop;
mov:
    goto loop;
io:
    goto loop;
halt:
#if(defined(TCVM_DEBUG_CPU))
    puts("halt.");
#endif
    exit(0);
error:
    exit(-2);
}

int main() {
    byte i = 0x1;
    mem_write_p(0, &i, byte);
    mainloop();
}
