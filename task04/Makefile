TARGET	= task04

SRC_DIR	= src
BIN_DIR	= bin

CFLAGS	= -c -Wall
SOURCES	= $(wildcard $(SRC_DIR)/*.c)
OBJECTS	= $(SOURCES:$(SRC_DIR)/%.c=$(BIN_DIR)/%.o)


all: $(BIN_DIR)/$(TARGET)
rebuild: clean all


$(BIN_DIR)/$(TARGET): $(OBJECTS)
	gcc  $(OBJECTS) -o $(BIN_DIR)/$(TARGET)
	@echo "Linking complete!"

# object files generation from sources
$(BIN_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -o $@ $<

.PHONY: clean
clean:
	rm -rf $(BIN_DIR)/$(TARGET) $(BIN_DIR)/*.o
	@echo "Cleanup complete!"