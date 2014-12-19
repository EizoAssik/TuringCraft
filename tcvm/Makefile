CFLAGS=-g -Iinclude -Wall
vpath %.c src

vm: cpu.c mem.c tty.c vm.c
	$(CC) $(CFLAGS) -DTCVM_DEBUG_CPU $^

cpu: cpu.c mem.c tty.c
	$(CC) $(CFLAGS) -DTCVM_DEBUG_CPU $^

mem: mem.c
	$(CC) $(CFLAGS) -DTCVM_DEBUG_MEM $^
	
.PHONY : clean

clean:
	-rm -rf a.out* 
