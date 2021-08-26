obj-m += ex02.o test_ex02.o
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
test:
	sudo dmesg -C
	sudo insmod ex02.ko
	- sudo insmod test_ex02.ko
	sudo rmmod ex02.ko
	dmesg