TuringCraft - VM
================

TuringCraft的虚拟机TCMachine包括：

+   8KB BOOTLOADER
+  64MB RAM
+ 128MB STORAGE, mapped in memory at high address.
+   1   64-bit CPU
+   1   KEYBOARD
+   1   TTY

其中，CPU详细定义如下

+ 8 64-bit register: A-H, X: 64-bit, H/L: 32-bit
+ 1 PC register
+ 1 PWD register
+ 1 instruction per cycle
+ 1 or 2 byte per instruction
+ NO ANY MEMORY MANAGING SUPPORT
    <-指令|设置-><-第1操作数|第2操作数->
    +-*/%<<>>&|!~
    IORWJHN
    00000|000 -- --- --- NOP
    00000|001 -- --- --- HALT
    00001|SET -- --- REG JUMP      100 010 001 < = >, 可加, 000 -> NOP, 111 -> JMP
    -----+-------------------
    00101|REG -- --- --- NOT
    00110|REG -- --- --- REV
    -----+-------------------
    01000|SET -- REG REG +         1S0 0S1 float integer, S -> singled
    01001|SET -- REG REG -         1S0 0S1 float integer
    01010|SET -- REG REG *         1S0 0S1 float integer 
    01011|SET -- REG REG /         1S0 0S1 float integer        
    01100|--- -- REG REG %         must be integers
    01101|SET -- REG REG <<>>      100 001 << >>, logical
    01110|--- -- REG REG &         must be integers
    01111|--- -- REG REG |         must be integers
    -----+-------------------
    10100|--- -- REG REG CMP       
    10000|REG -- REG SET READ-MEM  SET: 0,1,2,3->reg 4,5,6,7->Imm
    10001|REG -- REG SET SEND-MEM
    10010|REG -- REG SET READ-DEV
    10011|REG -- REG SET SEND-DEV
    -----+-------------------
    11000|REG -- --- REG REG-COPY
    11001|REG -- --- SET REG-SET 

    PWD, 8-bits:
    -|-|-|-|-|L|E|G
