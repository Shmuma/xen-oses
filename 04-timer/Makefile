CPPFLAGS += -I/usr/src/linux/include/xen/interface -Iinclude
LDFLAGS  += -nostdlib -T example.lds
CFLAGS   += -std=c99 -m32 -D__XEN_INTERFACE_VERSION__=0x00030203
ASFLAGS   = -D__ASSEMBLY__ -m32

.PHONY: all

all: testkernel

testkernel: bootstrap.x86_32.o kernel.o console.o event.o timer.o
	$(CC) $(LDFLAGS) $^ -o testkernel

clean:
	rm -f *.o 
	rm -f testkernel
