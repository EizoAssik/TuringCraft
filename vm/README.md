TuringCraft - VM
================

TuringCraft世界的虚拟机是一台很不科学的机器。它有着毫无逻辑的指令集和不合常理的硬件设置。开机，停机，天空有骆驼飞过。

TuringCraft的虚拟机TCVM包括：

+  8KB BOOTLOADER
+ 64MB RAM
+ 16MB HDD (not yet)
+  1   64-bit CPU
+  1   KEYBOARD
+  1   TTY

其中，TCVM的CPU描述如下：

+ 8x 64位通用寄存器   A-H
+ 1x 程序计数器(PC)   仅能通过JUMP指令修改
+ 1x 程序状态字(PWD)  执行比较指令时修改
+ 1x 指令/周期
+ 不提供任何内存抽象

## TCVM 机器语言

>   格式: <-指令|设置->[<-第1操作数|第2操作数->]
>   00000|000 -- --- --- NOP       空操作
>   00000|001 -- --- --- HALT      停机，程序必须已HALT结束
>   00001|SET -- --- REG JUMP     \<, =, \> 一次标记为 4,2,1, 可加, 0x7恒跳转
>   -----+-------------------
>   00101|REG -- --- --- NOT       逻辑非
>   00110|REG -- --- --- REV       按位取反
>   -----+-------------------
>   01000|FSI -- REG REG +         FSI，S标记是否使用符号位，F/I标记浮点/整数，下同
>   01001|FSI -- REG REG -         
>   01010|FSI -- REG REG *          
>   01011|FSI -- REG REG /                 
>   01100|--- -- REG REG %         寄存器中的值被理解为整数 
>   01101|SET -- REG REG SHIFT     逻辑位移，100/001标示左移/右移
>   01110|--- -- REG REG &         必须为整数 
>   01111|--- -- REG REG |         必须为整数
>   -----+-------------------
>   10100|--- -- REG REG CMP       
>   10000|REG -- REG WTH READ-MEM WTH%4标记宽度 0/1/2/3标示8/16/32/64位
>   10001|REG -- REG WTH SEND-MEM WTH小于4使用寄存器，否则使用立即数
>   10010|REG -- REG WTH READ-DEV
>   10011|REG -- REG WTH SEND-DEV
>   -----+-------------------
>   11000|REG -- --- REG REG-COPY
>   11001|REG -- --- WTH REG-SET 

>    PWD 目前仅使用低3位，标记比较结果
>    -|-|-|-|-|L|E|G
>    立即数按照字节序列写入指令流
