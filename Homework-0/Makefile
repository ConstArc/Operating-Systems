CC = gcc
CFLAGS = -Wall -Werror -Wextra
SRC_DIR = src
INCLUDE_DIR = include
OBJ_DIR = output
LOGS_DIR = logs
EXE = $(OBJ_DIR)/mvote

# Change the name of the test file for initial file insertion here.
# Since given instructions from piazza, suggest that we don't have any test files in the final project,
# this is only an example and won't work without adding the particular file in the tests folder.
FILE_NAME = ./tests/voters100000.csv

# Change the bucket size here
B_VALUE = 2

# Change the initial size of the hash table here
M_VALUE = 2


SRC_FILES = $(wildcard $(SRC_DIR)/*.c)

OBJ_FILES = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC_FILES))

INCLUDE_FLAGS = -I$(INCLUDE_DIR)

all: $(EXE)

$(EXE): $(OBJ_FILES)
	$(CC) $(CFLAGS) $(INCLUDE_FLAGS) -o $@ $(OBJ_FILES)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDE_FLAGS) -c $< -o $@

run:
	./$(EXE) -f $(FILE_NAME) -b $(B_VALUE) -m $(M_VALUE) 2> ./$(LOGS_DIR)/Errors.txt

valgrind: $(EXE)
	valgrind --leak-check=full --log-file=./$(LOGS_DIR)/Leaks.txt ./$(EXE) -f $(FILE_NAME) -b $(B_VALUE) -m $(M_VALUE) 2> ./$(LOGS_DIR)/Errors.txt

clean:
	rm -rf $(OBJ_FILES) $(EXE) $(LOGS_DIR)/*