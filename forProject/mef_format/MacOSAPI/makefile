default:
	clang -arch arm64 -c start.asm -o start.o && \
	clang -arch arm64 -nostdlib -O2 -c mef_format.c -o mef_format.o && \
	ld -arch arm64 -e _start -no_pie -image_base 0x100000 -o mef_loader start.o mef_format.o -L$$(xcrun --show-sdk-path)/usr/lib -lSystem && \
	codesign -s - --entitlements entitlements.plist -f mef_loader && \
	rm -f start.o mef_format.o

gen_test_mef: gen_test_mef.c mef_format.h
	clang -O2 -o gen_test_mef gen_test_mef.c

test.mef: gen_test_mef
	./gen_test_mef

clean:
	rm -f *.o mef_loader gen_test_mef test.mef

run: default test.mef
	./mef_loader test.mef
