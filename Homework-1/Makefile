CC = gcc
CFLAGS = -Wall -Werror -Wextra -g

##############################
### Change arguments below ###
##############################

# Change test file here
FILENAME = ./tests/voters100000.bin

# Change k flag here
K_CHILDREN = 10

# Change e1 flag here
S1_ALGO = heapsort

# Change e2 flag here
S2_ALGO = mergesort


SRC_DIR    = ./src
OUTPUT_DIR = ./output
LOGS_DIR   = ./logs
VALOUT_DIR = ./valout

EXECS = mysort workloader sorterM sorterH
OBJS  = record inputUtils processUtils logs

.PHONY: all run valgrind clean

all: $(EXECS:%=$(OUTPUT_DIR)/%)

$(OUTPUT_DIR)/mysort: $(SRC_DIR)/coordinator.c $(OBJS:%=$(OUTPUT_DIR)/%.o)
	$(CC) $^ -o $@ $(CFLAGS)

$(OUTPUT_DIR)/workloader: $(SRC_DIR)/workloader.c $(OBJS:%=$(OUTPUT_DIR)/%.o)
	$(CC) $^ -o $@ $(CFLAGS)

$(OUTPUT_DIR)/sorterM: $(SRC_DIR)/mergesort.c $(OBJS:%=$(OUTPUT_DIR)/%.o)
	$(CC) $^ -o $@ $(CFLAGS)

$(OUTPUT_DIR)/sorterH: $(SRC_DIR)/heapsort.c $(OBJS:%=$(OUTPUT_DIR)/%.o)
	$(CC) $^ -o $@ $(CFLAGS)

$(OUTPUT_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) -c $< -o $@ $(CFLAGS)

run: clean all
	mkdir -p $(LOGS_DIR)
	$(OUTPUT_DIR)/mysort -i "$(FILENAME)" -k $(K_CHILDREN) -e1 "$(S1_ALGO)" -e2 "$(S2_ALGO)" 2> $(LOGS_DIR)/Logs.txt

valgrind: clean all
	mkdir -p $(VALOUT_DIR)
	mkdir -p $(LOGS_DIR)
	valgrind --leak-check=full --show-leak-kinds=all --trace-children=yes \
			--log-file='$(VALOUT_DIR)/%p.log' \
			$(OUTPUT_DIR)/mysort -i "$(FILENAME)" -k $(K_CHILDREN) -e1 "$(S1_ALGO)" -e2 "$(S2_ALGO)" 2> $(LOGS_DIR)/Logs.txt

clean:
	rm -f $(OUTPUT_DIR)/* $(LOGS_DIR)/* $(VALOUT_DIR)/* 
