
ifeq ($(KERNEL_DIR),)
	KERNEL_DIR = /lib/modules/$(shell uname -r)/build
endif

TARGET = mem_test

PROJECT_DIR=$(shell pwd)

SRC_DIR	= $(PROJECT_DIR)/src
BIN_DIR	= $(PROJECT_DIR)/build


.PHONY: all build sub-make clean

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


send: build
	@scp -P 8022 $(BIN_DIR)/$(TARGET).ko user@localhost:~