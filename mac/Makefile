default:
	nasm -f macho64 code.asm -o code.o && ld code.o -o code -e _main

run: default
	./code