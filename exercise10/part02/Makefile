
TARGET = string_processor

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

test: all
	@sudo dmesg -C
	@sudo insmod $(BIN_DIR)/$(TARGET).ko
	@dmesg