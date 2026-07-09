CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -Werror
AR ?= ar
PREFIX ?= $(CURDIR)/dist

all: liblist_stack.a list_stack_demo list_stack_tests

list_stack.o: list_stack.c list_stack.h
	$(CC) $(CFLAGS) -c list_stack.c -o list_stack.o

list_stack_demo.o: list_stack_demo.c list_stack.h
	$(CC) $(CFLAGS) -c list_stack_demo.c -o list_stack_demo.o

list_stack_tests.o: list_stack_tests.c list_stack.h
	$(CC) $(CFLAGS) -c list_stack_tests.c -o list_stack_tests.o

liblist_stack.a: list_stack.o
	$(AR) rcs liblist_stack.a list_stack.o

list_stack_demo: list_stack_demo.o liblist_stack.a
	$(CC) $(CFLAGS) list_stack_demo.o liblist_stack.a -o list_stack_demo

list_stack_tests: list_stack_tests.o liblist_stack.a
	$(CC) $(CFLAGS) list_stack_tests.o liblist_stack.a -o list_stack_tests

run: demo

demo: list_stack_demo
	./list_stack_demo

test: list_stack_tests
	./list_stack_tests

install: liblist_stack.a list_stack.h
	mkdir -p "$(PREFIX)/include" "$(PREFIX)/lib"
	cp list_stack.h "$(PREFIX)/include/list_stack.h"
	cp liblist_stack.a "$(PREFIX)/lib/liblist_stack.a"
