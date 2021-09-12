PWD = $(shell pwd)

ifeq ($(KERNEL_DIR),)
	KERNEL_DIR = /lib/modules/$(shell uname -r)/build
endif

TARGET = txt_messenger
obj-m   := $(TARGET).o

all: clean_ko default clean

default:
	@echo "KERNEL_DIR: ${KERNEL_DIR}"
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) modules

clean:
	#$(MAKE) -C $(KERNEL_DIR) M=$(PWD) clean
	@rm -f *.o .*.cmd .*.flags *.mod*  *.order *.symvers *~
	@rm -fR .tmp*

clean_ko:
	@rm -f *.ko