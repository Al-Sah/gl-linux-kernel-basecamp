CURRENT = $(shell uname -r)
KDIR = /lib/modules/$(CURRENT)/build
PWD = $(shell pwd)
DEST = /lib/modules/$(CURRENT)/misc
MAJOR = 201

TARGET1 = fixdev
TARGET2 = dyndev
obj-m   := $(TARGET1).o $(TARGET2).o

all: default clean

default:
	@$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	@rm -f *.o .*.cmd .*.flags *.mod *.mod.c *.order
	@rm -f .*.*.cmd *.symvers *~ *.*~ TODO.*
	@rm -fR .tmp*
	@rm -rf .tmp_versions

clean_all: clean
	@rm -f *.ko
