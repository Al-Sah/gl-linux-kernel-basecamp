PWD = $(shell pwd)

ifeq ($(KERNEL_DIR),)
	KERNEL_DIR = /lib/modules/$(shell uname -r)/build
endif

obj-m += ex02.o test_ex02.o

all: clean
	@echo "KERNEL_DIR: ${KERNEL_DIR}"
	@$(MAKE) -C $(KERNEL_DIR) M=$(PWD) modules

clean:
	@$(MAKE) -C $(KERNEL_DIR) M=$(PWD) clean


test: all
	@sudo dmesg -C
	@insmod ex02.ko
	@- sudo insmod test_ex02.ko
	@sudo rmmod ex02
	@dmesg

test_remote: all
	@scp -P 8022 *.ko user@localhost:~
	@-ssh -p 8022 root@localhost "cd /home/user/; insmod ex02.ko; insmod test_ex02.ko; rmmod ex02.ko; dmesg | tail -10"