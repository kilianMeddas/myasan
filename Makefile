CC = gcc
CFLAGS = -std=c99 -pedantic -Werror -Wall -Wextra -Wvla -fPIC -Iinclude 
LDFLAGS = -shared
SRC = ....
SRC_H =....
OBJECT = $(SRC:.c=.o)

TARGET = libmyasan.so
TEST = tests/test_suite

.PHONY: all library check clean format debug

all: library

library: $(TARGET)

$(TARGET):$(OBJECT)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJECT)

format:
	clang-format -i $(SRC) $(SRC_H) $(TEST).c $(TEST).h

$(TEST): tests/test_suite.c $(OBJECT)
	$(CC) $(CFLAGS) tests/test_suite.c $(filter-out src/main.c, $(OBJECT)) -lcriterion -o $(TEST)

check: $(TARGET) $(TEST)
	./$(TEST)

debug: CFLAGS += -g
debug: clean
	$(MAKE) all

clean: 
	rm -f $(TARGET) $(OBJECT) $(TEST)

