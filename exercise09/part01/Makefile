
PWD = $(shell pwd)
ifeq ($(KERNEL_DIR),)
	KERNEL_DIR = /lib/modules/$(shell uname -r)/build
endif

obj-m += ex01.o

all: clean
	@$(MAKE) -C $(KERNEL_DIR) M=$(PWD) modules

clean:
	@$(MAKE) -C $(KERNEL_DIR) M=$(PWD) clean

test: all
	@sudo dmesg -C
	@sudo insmod ex01.ko
	@sudo rmmod ex01.ko
	@dmesg

test_remote: all
	@scp -P 8022 *.ko user@localhost:~
	@-ssh -p 8022 root@localhost "cd /home/user/; insmod ex01.ko; rmmod ex01.ko; dmesg | tail -10"