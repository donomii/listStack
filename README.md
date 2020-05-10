# listStack
 A linked list implemented sequentially in memory
 
## Features
 A linked list where all the elements are laid out sequentially in memory,                                               to avoid malloc and free.  The elements can be variable size.

 Why?  To provide a linked list implementation that works in embedded systems, without needing                           memory management.

 To use this, you must first allocate a chunk of memory, then pass it
 to new_list_stack.  new_list_stack creates a small header structure at
 the start of that memory, and the rest is used for the list.         

 # Examples

	  char mem[1000];
	  list_stack ls   = new_list_stack(mem, 1000);
	  const char  hello_world[]  = "Hello World.   ";
	  const char  greet[] =        "How are you today? ";

 
	  char * target = (char *)ls_cons_blank(strlen(greet)+1, ls);
	  strcpy(target, greet);
	  int strl = strlen(hello_world) + 1;
	  target = (char *)ls_cons_blank(strl, ls);
	  strcpy(target, hello_world);

	  for (ptr i = ls_start(ls); !ls_is_end(i); i=ls_tail(i)){
	    printf("%s",(char *)ls_head(i));
	  }
	  printf("\n");	
