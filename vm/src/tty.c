#include <stdio.h>
#include "common.h"
#include "tty.h"

void tty_write_byte(byte value) {
    putchar(value);
}

void tty_reset() {
    fputs("\033[A\033[2K\033[A\033[2K",stdout);
}

