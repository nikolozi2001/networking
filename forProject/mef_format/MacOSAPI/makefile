default:
	nasm -f elf64 start.asm -o start.o && \
	clang -nostdlib -static -O2 -c mef_format.c -o mef_format.o && \
	ld -macosx_version_min 12.0 -e _start -o mef_loader start.o mef_format.o && \
	rm -f start.o mef_format.o

clean:
	rm -f *.o

run: default
	./mef_loader
