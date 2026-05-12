obj-m += mef_format.o

all:	
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean	
	cp .vscode/compile_commands.json compile_commands.json

info:
	modinfo mef_format.ko

load:
	sudo insmod mef_format.ko

msg:
	sudo dmesg | tail -n 30

msg_clean:
	sudo dmesg -C

find:
	sudo lsmod | grep mef_format

unload:
	sudo rmmod mef_format

install:
	sudo apt update -y && sudo apt install linux-headers-$(uname -r) -y

