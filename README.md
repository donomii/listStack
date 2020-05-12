# listStack
 A linked list implemented sequentially in memory
 
## Features
 A linked list where all the elements are laid out sequentially in memory,                                               to avoid malloc and free.  The elements can be variable size.

 Why?  To provide a linked list implementation that works in embedded systems, without needing                           memory management.

 To use this, you must first allocate a chunk of memory, then pass it
 to new_list_stack.  new_list_stack creates a small header structure at
 the start of that memory, and the rest is used for the list.         

 # Example

	  char mem[1000];
	  list_stack ls   = new_list_stack(mem, 1000);
	  const char  hello_world[]  = "Hello World.   ";
	  const char  greet[] =        "How are you today? ";

	  //Allocate list element, we write into the empty space 
	  char * target = (char *)ls_cons_blank(strlen(greet)+1, ls);
	  strcpy(target, greet);

	  //Allocate new list element, and automatically copy the source data into the list_stack
	  ls_cons((void *)hello_world, strlen(hello_world) + 1, ls);

	  for (ptr i = ls_start(ls); !ls_is_end(i); i=ls_tail(i)){
	    //ls_head returns a pointer to the data for element i
	    warn("%s", (char *)ls_head(i));
	  }
	  warn("\n");
