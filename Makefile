
all:
	gcc -o list_stack main.c -DTEST
test:
	./list_stack
