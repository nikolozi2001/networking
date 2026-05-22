obj-m += mef_loader.o

KDIR ?= /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

load:
	sudo insmod mef_loader.ko

unload:
	sudo rmmod mef_loader

find_mod:
	sudo lsmod | grep mef_loader

info:
	modinfo mef_loader.ko

msg_clean:
	sudo dmesg -c

msg:
	sudo dmesg | tail -n 10
