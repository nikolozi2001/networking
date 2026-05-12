obj-m += my_modute.o

all:	
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean	
	cp .vscode/compile_commands.json compile_commands.json
info:
	modinfo my_modute.ko

load:
	sudo insmod my_modute.ko

msg:
	sudo dmesg | tail

msg_clean:
	sudo dmesg -C

find:
	sudo lsmod | grep my_modute

unload:
	sudo rmmod my_modute

install:
	sudo apt update -y && sudo apt install linux-headers-$(uname -r) -y

