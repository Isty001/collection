CFLAGS := -std=gnu89 -g -Wall -Wextra -ftrapv -Wshadow -Wundef -Wcast-align -Wunreachable-code
TEST_SRC = src/*.c test/*.c

.PHONY: test

test:
	$(CC) $(CFLAGS) $(TEST_SRC) -o test.o
	./test.o

test-valgrind:
	make test
	valgrind --track-origins=yes --leak-check=full --show-reachable=yes ./test.o
