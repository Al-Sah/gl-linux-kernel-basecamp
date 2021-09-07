#
# procfs example
#

TARGET = procfs_rw
SRC_DIR	= src
BIN_DIR	= bin
SOURCES = $(wildcard $(SRC_DIR)/*.c)

ifeq ($(KERNEL_DIR),)
	KERNEL_DIR = /lib/modules/$(shell uname -r)/build
endif

.PHONY: all build rebuild clean test test_remote

build: $(BIN_DIR)/Makefile $(subst $(SRC_DIR),$(BIN_DIR),$(SOURCES))
	@echo "  Building...  KERNEL_DIR = ${KERNEL_DIR}"
	@make -C $(KERNEL_DIR) M=$(PWD)/$(BIN_DIR) modules

all: rebuild
rebuild: clean build

clean:
	@echo "  Cleaning... "
	@rm -rf $(BIN_DIR)
	@mkdir $(BIN_DIR)

test_remote: all
	scp -P 8022 $(BIN_DIR)/*.ko user@localhost:~


test: all
	sudo dmesg -C
	sudo insmod procfs_rw.ko
	sudo rmmod procfs_rw.ko
	dmesg


$(BIN_DIR)/%: $(SRC_DIR)/% # Create a symlink from src to bin
	-@ln -s ../$< $@
$(BIN_DIR)/Makefile: # Generate a Makefile with the needed obj-m
	@echo "obj-m += $(TARGET).o" > $@
