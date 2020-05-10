
all:
	gcc -o list_stack list_stack.c -DTEST
test:
	./list_stack
