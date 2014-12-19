#include <stdio.h>
#include <stdlib.h>

#include "mem.h"
#include "tty.h"
#include "cpu.h"

#define TCVM_CPU_REGISTERS 8

static ui64 PC = 0;
static void * registers[TCVM_CPU_REGISTERS];
static ui8 PWD = 0;

#define ins_in_reg(ins) (*((byte *)(registers[ins&0x07])))
#define reg_of(ins, type) (*((type *)(registers[ins&0x07])))

#define get_ins()      mem_read(PC, byte)
#define get_next_ins() mem_read(PC++, byte)

static inline void chpwd_cmp(byte args) {
    ui64 m, n;
    n = reg_of(args, ui64);
    m = reg_of(args >> 3, ui64);
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
            case 0: reg_of(arg>>3, ui64) %= reg_of(arg, ui64); break;
            case 1:
                    if ((ins & 0x7) == 1) {
                        reg_of(arg>>3, ui64) >>= reg_of(arg, ui64);
                    } else {
                        reg_of(arg>>3, ui64) <<= reg_of(arg, ui64);
                    }
                    break;
            case 2: reg_of(arg>>3, ui64) &= reg_of(arg, ui64); break;
            case 3: reg_of(arg>>3, ui64) |= reg_of(arg, ui64); break;
        }
    } else {
        if ((ins & 0x7) == 0x1) { /* integers, unsigned */
            switch ((ins >> 3) & 0x3) {
                case 0: reg_of(arg>>3, ui64) += reg_of(arg, ui64); break;
                case 1: reg_of(arg>>3, ui64) -= reg_of(arg, ui64); break;
                case 2: reg_of(arg>>3, ui64) *= reg_of(arg, ui64); break;
                case 3: reg_of(arg>>3, ui64) /= reg_of(arg, ui64); break;
            }
        } else if ((ins & 0x7) == 0x3) { /* integers, signed */
            switch ((ins >> 3) & 0x3) {
                case 0: reg_of(arg>>3, i64) += reg_of(arg, i64); break;
                case 1: reg_of(arg>>3, i64) -= reg_of(arg, i64); break;
                case 2: reg_of(arg>>3, i64) *= reg_of(arg, i64); break;
                case 3: reg_of(arg>>3, i64) /= reg_of(arg, i64); break;
            }
        } else { /* floats must be signed */
            switch ((ins >> 3) & 0x3) {
                case 0: reg_of(arg>>3, f64) += reg_of(arg, f64); break;
                case 1: reg_of(arg>>3, f64) -= reg_of(arg, f64); break;
                case 2: reg_of(arg>>3, f64) *= reg_of(arg, f64); break;
                case 3: reg_of(arg>>3, f64) /= reg_of(arg, f64); break;
            }
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
            mem_write_p(reg_of(arg >> 3, ui64), imm, ui64);
        } else { /* read */
            reg_of(ins, ui64) = *(ui64*) mem_read_n(imm, len);
        }
    } else { /* using registers */
        if (ins & 0x08) { /* write */
            mem_write_n(reg_of(arg >> 3, ui64), &reg_of(arg, ui64), len);
        } else { /* read */
            reg_of(ins, ui64) = *(ui64*) mem_read_n(reg_of(arg >> 3, ui64), len);
        }
    }
}

void mainloop() {
    ui64 imm;
    byte ins;
    while(1) {
loop:
        ins = get_next_ins();
        /* First router */
        switch(ins >> 6) {
            case 0:
                if(ins == 0x00) break;
                if(ins == 0x01) goto halt;
                if(ins &  0x08) goto jump;
                if(ins &  0x20) goto sop;
                goto error;
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
        imm = _read_imm(get_next_ins());
        reg_of(ins, ui64) = imm;
    } else { /* reg-copy */
        reg_of(ins, ui64) = reg_of(get_next_ins(), ui64);
    }
    goto loop;
sop:
    if ((ins & 0x18) == 1) {
        reg_of(ins, ui64) = !reg_of(ins, ui64);
    } else {
        reg_of(ins, ui64) = ~reg_of(ins, ui64);
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
        PC = reg_of(get_next_ins(), ui64);
    else
        PC++;
    goto loop;
mov:
    _mov(ins, get_next_ins());
    goto loop;
io:
    if (ins & 0x08) { /* write to divices */
        tty_write_byte(reg_of(ins, ui64) & 0xFF); 
    } else { /* read from devices */
        reg_of(ins, ui64) = getchar();
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

void cpu_init() {
    /* 随机初始化 */
    for (int i = 0; i != TCVM_CPU_REGISTERS; i++) {
        registers[i] = malloc(sizeof(ui64));
    }
}

