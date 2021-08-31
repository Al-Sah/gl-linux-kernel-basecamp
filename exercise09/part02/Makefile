obj-m += ex02.o test_ex02.o

build_remote:
	export BUILD_KERNEL=/home/al_sah/GL-linuxBaseCamp/linux-kernel/linux-5.13.12/x02
	make -C $(BUILD_KERNEL) M=$(PWD) modules

test_remote: build_remote
	scp -P 8022 *.ko user@localhost:~
	-ssh -p 8022 root@localhost "cd /home/user/; insmod ex02.ko; insmod test_ex02.ko; rmmod ex02.ko; dmesg | tail -10"

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules


clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean


test: all
	sudo dmesg -C
	sudo insmod ex02.ko
	- sudo insmod test_ex02.ko
	sudo rmmod ex02.ko
	dmesg