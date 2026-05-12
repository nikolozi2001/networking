default:
	as -o code.o code.asm && ld code.o -o code -e _main -lSystem -platform_version macos 14.0 15.0 -L$$(xcrun --show-sdk-path)/usr/lib
run:
	./code
clean:
	rm -f code code.o