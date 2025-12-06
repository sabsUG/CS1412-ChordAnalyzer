CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2

SRC_DIR = src
OBJ_DIR = obj

SRC = $(SRC_DIR)/polynizer.c \
      $(SRC_DIR)/dictADT.c \
      $(SRC_DIR)/trie.c \
      $(SRC_DIR)/read_file.c \
      $(SRC_DIR)/process_matrix.c

OBJ = $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

TARGET = polynizer

.PHONY: all clean dirs

all: dirs $(TARGET)

dirs:
	mkdir -p $(OBJ_DIR) out

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(TARGET)