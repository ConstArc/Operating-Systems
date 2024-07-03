CC = gcc
CFLAGS = -Wall -Wextra -lrt -lpthread -I./include -g -O3
ifdef DEBUG
	CFLAGS += -DDEBUG
endif

SRC_DIR    = ./src
LOG_DIR    = ./log
TESTS_DIR  = ./tests
BUILD_DIR  = ./bin
VALOUT_DIR = ./valout
SHMEM_FILENAME = shared_memory


# Change test file here
FILENAME = $(TESTS_DIR)/accounts50.bin

# Change n flag here
N_CHILDREN = 300


EXECS = initlzer reader writer accountant logger
OBJS  = record inputUtils processUtils loggingUtils shmem synchroUtils initlzerUtils

.PHONY: all run valgrind clean

all: $(EXECS:%=$(BUILD_DIR)/%)

clean_shmem:
	rm /dev/shm/$(SHMEM_FILENAME)

clean_tests:
	rm -rf $(TESTS_DIR)/*
	cp -r SampleData/* $(TESTS_DIR)

$(BUILD_DIR)/initlzer: $(SRC_DIR)/initlzer/initlzer.c $(OBJS:%=$(BUILD_DIR)/%.o) $(BUILD_DIR)/initlzerUtils.o
	$(CC) $^ -o $@ $(CFLAGS)

$(BUILD_DIR)/reader: $(SRC_DIR)/reader/reader.c $(OBJS:%=$(BUILD_DIR)/%.o) $(BUILD_DIR)/readerUtils.o
	$(CC) $^ -o $@ $(CFLAGS)

$(BUILD_DIR)/writer: $(SRC_DIR)/writer/writer.c $(OBJS:%=$(BUILD_DIR)/%.o) $(BUILD_DIR)/writerUtils.o
	$(CC) $^ -o $@ $(CFLAGS)

$(BUILD_DIR)/accountant: $(SRC_DIR)/logging/accountant.c $(OBJS:%=$(BUILD_DIR)/%.o) $(BUILD_DIR)/loggingUtils.o
	$(CC) $^ -o $@ $(CFLAGS)

$(BUILD_DIR)/logger: $(SRC_DIR)/logging/logger.c $(OBJS:%=$(BUILD_DIR)/%.o) $(BUILD_DIR)/loggingUtils.o
	$(CC) $^ -o $@ $(CFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/common/%.c
	$(CC) -c $< -o $@ $(CFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/initlzer/%.c
	$(CC) -c $< -o $@ $(CFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/reader/%.c
	$(CC) -c $< -o $@ $(CFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/writer/%.c
	$(CC) -c $< -o $@ $(CFLAGS)

$(BUILD_DIR)/initlzerUtils.o: $(SRC_DIR)/initlzer/initlzerUtils.c
	$(CC) -c $< -o $@ $(CFLAGS)

$(BUILD_DIR)/readerUtils.o: $(SRC_DIR)/reader/readerUtils.c
	$(CC) -c $< -o $@ $(CFLAGS)

$(BUILD_DIR)/writerUtils.o: $(SRC_DIR)/writer/writerUtils.c
	$(CC) -c $< -o $@ $(CFLAGS)

$(BUILD_DIR)/loggingUtils.o: $(SRC_DIR)/logging/loggingUtils.c
	$(CC) -c $< -o $@ $(CFLAGS)

run: all
	mkdir -p $(LOG_DIR)
	rm -f $(LOG_DIR)/*
	$(BUILD_DIR)/initlzer -f "$(FILENAME)" -n $(N_CHILDREN) 2> $(LOG_DIR)/Errors.log

run_logger:
	$(BUILD_DIR)/logger -s $(SHMEM_FILENAME)

valgrind: all
	mkdir -p $(VALOUT_DIR)
	mkdir -p $(LOG_DIR)
	rm -f $(VALOUT_DIR)/* $(LOG_DIR)/*
	valgrind --leak-check=full --show-leak-kinds=all --trace-children=yes \
			--log-file='$(VALOUT_DIR)/%p.log' \
			$(BUILD_DIR)/initlzer -f "$(FILENAME)" -n $(N_CHILDREN) 2> $(LOG_DIR)/Errors.log

clean:
	rm -f $(BUILD_DIR)/* $(VALOUT_DIR)/* $(LOG_DIR)/*