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
	$(CC) $< -L"$(CURDIR)"  -lmyasan -Wl,-rpath,"$(CURDIR)" -o $@


check: $(TARGET) $(TEST_BIN)
	@printf "\n"
	@failed=0; \
	for test in $(TEST_BIN); do \
		echo "Running $$test"; \
		LD_PRELOAD="$(CURDIR)/$(TARGET)" ./$$test; \
		result=$$?; \
		expected=0; \
		case "$$test" in \
			tests/errors/test_double_free|tests/errors/test_double_free_alias) expected=4 ;; \
			tests/errors/test_invalid_*) expected=1 ;; \
			tests/errors/test_overflow*) expected=3 ;; \
			tests/errors/test_use_after_free*) expected=2 ;; \
		esac; \
		echo "Exit code: $$result (expected: $$expected)"; \
		if [ "$$result" -eq "$$expected" ]; then \
			echo "PASS"; \
		else \
			echo "FAIL"; \
			failed=1; \
		fi; \
		echo ""; \
	done; \
	exit $$failed

debug: CFLAGS += -g
debug: clean
	$(MAKE) all

clean: 
	rm -f $(TARGET) $(OBJECT) $(TEST_BIN)

