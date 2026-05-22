default:
	nasm -f elf64 start.asm -o start.o && \
	clang -nostdlib -static -O2 -c mef_format.c -o mef_format.o && \
	ld -T loader.ld -o mef_loader start.o mef_format.o && \
	nasm -f bin mini64.asm -o mini64.mef && chmod u+x mini64.mef && \
	rm -f start.o mef_format.o

clean:
	rm -f *.o
	rm -f mini64.mef
	rm -f mef_loader

run: default
	./mef_loader mini64.mef