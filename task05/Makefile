
TARGET  = task05

PROJECT_DIR=$(shell pwd)
SRC_DIR	= $(PROJECT_DIR)/src
BIN_DIR	= $(PROJECT_DIR)/build
SOURCES = $(wildcard $(SRC_DIR)/*.c)

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

send: all
	@scp -P 8022 $(BIN_DIR)/*.ko user@localhost:~

test: all
	@sudo dmesg -C
	@sudo insmod $(BIN_DIR)/$(TARGET).ko money=200 in_currency="EUR" out_currency="UAH"
	@# sudo insmod task05.ko money=200
	@sudo rmmod task05.ko
	@dmesg