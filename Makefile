CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -Werror
CXX ?= c++
CXXFLAGS ?= -std=c++11 -Wall -Wextra -Wpedantic -Werror
ARDUINO_CLI ?= arduino-cli
ARDUINO_FQBN ?= arduino:avr:uno
AR ?= ar
PREFIX ?= $(CURDIR)/dist

.PHONY: all run demo test test-arduino install

all: liblist_stack.a list_stack_demo list_stack_tests

list_stack.o: list_stack.c list_stack.h
	$(CC) $(CFLAGS) -c list_stack.c -o list_stack.o

list_stack_demo.o: list_stack_demo.c list_stack.h list_stack_demo.h
	$(CC) $(CFLAGS) -c list_stack_demo.c -o list_stack_demo.o

list_stack_tests.o: list_stack_tests.c list_stack.h
	$(CC) $(CFLAGS) -c list_stack_tests.c -o list_stack_tests.o

liblist_stack.a: list_stack.o
	$(AR) rcs liblist_stack.a list_stack.o

list_stack_demo: list_stack_demo.o liblist_stack.a
	$(CC) $(CFLAGS) list_stack_demo.o liblist_stack.a -o list_stack_demo

list_stack_tests: list_stack_tests.o liblist_stack.a
	$(CC) $(CFLAGS) list_stack_tests.o liblist_stack.a -o list_stack_tests

list_stack_cpp_tests: list_stack_cpp_tests.cpp list_stack.h liblist_stack.a
	$(CXX) $(CXXFLAGS) list_stack_cpp_tests.cpp liblist_stack.a -o list_stack_cpp_tests

run: demo

demo: list_stack_demo
	./list_stack_demo

test: list_stack_tests list_stack_cpp_tests
	./list_stack_tests
	./list_stack_cpp_tests

test-arduino:
	mkdir -p build/arduino/list_stack_demo
	cp list_stack.c list_stack.h list_stack_demo.c list_stack_demo.h list_stack_arduino.cpp build/arduino/list_stack_demo/
	printf '%s\n' '#include "list_stack_demo.h"' > build/arduino/list_stack_demo/list_stack_demo.ino
	$(ARDUINO_CLI) compile --fqbn $(ARDUINO_FQBN) --warnings all --build-path build/arduino/output build/arduino/list_stack_demo

install: liblist_stack.a list_stack.h
	mkdir -p "$(PREFIX)/include" "$(PREFIX)/lib"
	cp list_stack.h "$(PREFIX)/include/list_stack.h"
	cp liblist_stack.a "$(PREFIX)/lib/liblist_stack.a"
