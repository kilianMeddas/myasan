CC = gcc
CFLAGS = -std=c99 -pedantic -Werror -Wall -Wextra -Wvla -fPIC -Iinclude 
LDFLAGS = -shared
SRC = $(wildcard src/*.c)
SRC_H = $(wildcard include/*.h)

OBJECT = $(SRC:.c=.o)

TARGET = libmyasan.so

TEST_SRC = $(shell find tests -type f -name '*.c')
TEST_H = $(shell find tests -type f -name '*.h')
TEST_BIN = $(TEST_SRC:.c=)

.PHONY: all library check clean format debug

all: library

library: $(TARGET)

$(TARGET):$(OBJECT)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJECT)

format:
	clang-format -i $(SRC) $(SRC_H) $(TEST_SRC) $(TEST_H)


tests/%: tests/%.c $(TARGET)
	$(CC) $(CFLAGS) $< -L"$(CURDIR)" -lcriterion -lmyasan -Wl,-rpath,"$(CURDIR)" -o $@


check: clean $(TARGET) $(TEST_BIN)
	@printf "\n\n"
	@for test in $(TEST_BIN); do \
		echo "Running $$test"; \
		LD_PRELOAD="$(CURDIR)/$(TARGET)" ./$$test; \
		result=$$?; \
		echo "Exit code: $$result"; \
		if [ $$result -ne 0 ]; then exit $$result; fi; \
	done

debug: CFLAGS += -g
debug: clean
	$(MAKE) all

clean: 
	rm -f $(TARGET) $(OBJECT) $(TEST_BIN)

