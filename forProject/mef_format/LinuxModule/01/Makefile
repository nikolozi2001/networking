obj-m += hello.o

KDIR ?= /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	cp .vscode/compile_commands.json compile_commands.json

load:
	sudo insmod hello.ko

unload:
	sudo rmmod hello

find_mod:
	sudo lsmod | grep hello

info:
	modinfo hello.ko

msg_clean:
	sudo dmesg -c

msg:
	sudo dmesg | tail -n 10

install:
	sudo apt update -y && sudo apt upgrade -y && sudo apt install build-essential linux-headers-$(uname -r) -y