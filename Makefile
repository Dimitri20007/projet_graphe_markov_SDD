# ===== CONFIG =====
CC = gcc
CFLAGS = -Wall -Wextra -g -O2 -Iinclude -finput-charset=UTF-8 -fexec-charset=UTF-8 -DUNICODE -D_UNICODE
SRC_DIR = src
OBJ_DIR = obj
BIN = markov.exe
SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC))

# ===== RÈGLE PAR DÉFAUT =====
all: $(BIN)

# ===== Compilation programme =====
$(BIN): $(OBJ)
	if not exist $(OBJ_DIR) mkdir $(OBJ_DIR)
	$(CC) $(CFLAGS) -o $@ $^ -lm

# ===== Compilation des .c en .o =====
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	if not exist $(OBJ_DIR) mkdir $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# ===== Nettoyage =====
clean:
	if exist $(OBJ_DIR) rmdir /s /q $(OBJ_DIR)
	if exist $(BIN) del $(BIN)