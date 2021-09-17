
ifeq ($(KERNEL_DIR),)
	KERNEL_DIR = /lib/modules/$(shell uname -r)/build
endif

ifeq ($(TARGET),)
	TARGET = mytimer # Default target
endif

PROJECT_DIR=$(shell pwd)

SRC_DIR	= $(PROJECT_DIR)/src
BIN_DIR	= $(PROJECT_DIR)/build
SOURCES	= $(wildcard $(SRC_DIR)/*.c)
OBJECTS	= $(SOURCES:$(SRC_DIR)/%.c=$(BIN_DIR)/%.o)



.PHONY: all build sub-make clean send_all build_all


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

send_all: build_all
	@scp -P 8022 $(BIN_DIR)/*.ko user@localhost:~


build_all: clean
	@echo "  Building all ...  KERNEL_DIR = ${KERNEL_DIR}"
	@echo "obj-m += $(OBJECTS)" > $(SRC_DIR)/Makefile
	@$(MAKE) -C $(KERNEL_DIR) M=$(SRC_DIR) modules
	@cp $(SRC_DIR)/*.ko $(BIN_DIR)
	@$(MAKE) -C $(KERNEL_DIR) M=$(SRC_DIR)  clean