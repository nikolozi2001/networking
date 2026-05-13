default:
	nasm -f elf64 start.asm -o start.o && \
	clang -nostdlib -static -O2 -c mef_format.c -o mef_format.o && \
	ld -T loader.ld -o mef_loader start.o mef_format.o && \
	rm -f start.o mef_format.o

clean:
	rm -f *.o

run: default
	./mef_loader mini64.mef
