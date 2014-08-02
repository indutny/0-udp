CFLAGS ?=
CFLAGS += -Wall -Wextra -Werror
CFLAGS += -g -O0

test: test.c
	$(CC) $(CFLAGS) $< -o $@
