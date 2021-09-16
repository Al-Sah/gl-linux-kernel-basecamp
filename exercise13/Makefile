
TARGET = mytimer

PROJECT_DIR=$(shell pwd)
SRC_DIR	= $(PROJECT_DIR)/src
BIN_DIR	= $(PROJECT_DIR)/build

ifeq ($(KERNEL_DIR),)
	KERNEL_DIR = /lib/modules/$(shell uname -r)/build
endif


.PHONY: all build sub-make clean test test_remote

all: build


build: clean sub-make
	@echo "  Building...  KERNEL_DIR = ${KERNEL_DIR}"
	@$(MAKE) -C $(KERNEL_DIR) M=$(SRC_DIR) modules
	@cp $(SRC_DIR)/$(TARGET).ko $(BIN_DIR)/$(TARGET).ko
	@$(MAKE) -C $(KERNEL_DIR) M=$(SRC_DIR)  clean

sub-make:
	@echo "obj-m += $(TARGET).o" > $(SRC_DIR)/Makefile

clean:
	@echo "  Cleaning... "
	@rm -rf $(BIN_DIR)
	@mkdir $(BIN_DIR)


send: build_all
	@scp -P 8022 $(BIN_DIR)/*.ko user@localhost:~


build_all: clean
	@echo "  Building all...  KERNEL_DIR = ${KERNEL_DIR}"
	@echo "obj-m += tick.o interv.o mytimer.o hrtimer.o" > $(SRC_DIR)/Makefile
	@$(MAKE) -C $(KERNEL_DIR) M=$(SRC_DIR) modules
	@cp $(SRC_DIR)/tick.ko $(BIN_DIR)/tick.ko
	@cp $(SRC_DIR)/interv.ko $(BIN_DIR)/interv.ko
	@cp $(SRC_DIR)/hrtimer.ko $(BIN_DIR)/hrtimer.ko
	@cp $(SRC_DIR)/mytimer.ko $(BIN_DIR)/mytimer.ko
	@$(MAKE) -C $(KERNEL_DIR) M=$(SRC_DIR)  clean
	

