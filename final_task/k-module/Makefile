
ifeq ($(KERNEL_DIR),)
	KERNEL_DIR = /lib/modules/$(shell uname -r)/build
endif

TARGET1 = matrix_controller
TARGET2 = mod_test
TREE_FILE = spidev_off

PROJECT_DIR=$(shell pwd)

SRC_DIR	= $(PROJECT_DIR)/src
BIN_DIR	= $(PROJECT_DIR)/build

.PHONY: all build sub-make clean send

all: build

build: clean sub-make
	@echo "  Building...  KERNEL_DIR = ${KERNEL_DIR}"
	@$(MAKE) -C $(KERNEL_DIR) M=$(SRC_DIR) modules
	@cp $(SRC_DIR)/*.ko $(BIN_DIR)/
	@$(MAKE) -C $(KERNEL_DIR) M=$(SRC_DIR)  clean


sub-make:
	@echo "obj-m += $(TARGET1).o $(TARGET2).o" > $(SRC_DIR)/Makefile


clean:
	@echo "  Cleaning... "
	@rm -rf $(BIN_DIR)
	@mkdir $(BIN_DIR)
	@rm -f $(TREE_FILE).dtbo


create_dev_tree:
	@dtc $(TREE_FILE).dts -O dtb >$(TREE_FILE).dtbo
	@sudo dtoverlay -d . $(TREE_FILE)

enable_dev_tree: create_dev_tree
	@sudo cp $(TREE_FILE).dtbo /boot/overlays/
	@echo "dtoverlay=$(TREE_FILE)" | sudo tee -a /boot/config.txt