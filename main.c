
/* list_stack - A linked list implemented sequentially in memory

  A linked list where all the elements are laid out sequentially in memory,
  to avoid malloc and free.

  Why?  To provide a linked list implementation that works in embedded systems, without needing
  memory management.



  To use this, you must first allocate a chunk of memory, then pass it
  to new_list_stack.  new_list_stack creates a small header structure at
  the start of that memory, and the rest is used for the list.

  Scheme and Lisp programmers will be familiar with the basic functions
  to access the list:

  car  - return the data from the first element
  head - return the data from the first element

  cdr  - return the rest of the list, starting with the second element
  tail - return the rest of the list, starting with the second element

  cons - add an element to the front of the list, returns a pointer to the start of the list
  cons_blank - add a new entry that is blank, return a pointer to the start of the blank data area

  pop - Discard the first element of the list (this is the stack part)

  start   - returns a pointer to the start of the list, when given the handle
  is_last - check if the current list element is the last element (see loop example)
  room_for - check to see if there is enough space left for your data


  list_stack stores some control information at the start of the memory, in a control block.  The pointer returned from 
  new_list_stack(mem, size) is NOT the start of the list, it is the pointer to the control block.  To get
  the start of the list, call start(handle)

  new_list_stack - returns a handle to the list_stack

Note:  list_stack stores pointers after the data area.  If you write into the data area and write past
the end of the allocated space, you will overwrite pointers and corrupt the list.  If your program starts
crashing while accessing the list, check for off-by-one errors.

*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef ARDUINO
#include <Arduino.h>
#define ALIGN 4
typedef ls_uint uint;  // intptr_t?
#define warn(...) Serial.print(__VA_ARGS__)
#else
#define warn(...) printf(__VA_ARGS__)
#define ALIGN 1
typedef   intptr_t ls_uint;
#endif

typedef void * ptr;
typedef struct ls_header
{
  int size;
  ptr head;
  ptr last;
} ls_header;
typedef ls_header * list_stack;

list_stack new_list_stack(void * mem, int size) {
   if ((ls_uint)mem %ALIGN !=0 ) {
    warn("Warning! Destination address not aligned!\n");
  }
  ls_header* ls = (ls_header *) mem;
  ls->size = size;
  ls->last = NULL;
  ls->head = &ls->last;
  return ls;
}

ptr ls_start (list_stack ls) {
  return ls->head;
}

ptr ls_car(ptr h) {
  ptr ** head = (ptr **) h;
  ptr next = *head;
  if (next == NULL) {
    return NULL;
  }
  ptr data = next + sizeof(ptr);
  return data;
}

ptr ls_head(ptr h) {
  return ls_car(h);
}

ptr ls_cdr(ptr h) {
  ptr ** head = (ptr **) h;
  ptr next = *head;
  return next;
}


int ls_is_end(ptr h) {
  return ls_cdr(h) == NULL;
}

ptr ls_tail(ptr h) {
  return ls_cdr(h);
}

int ls_room_for(int size, list_stack ls){
  return (ptr )(ls +ls->size) > (ptr )(ls->head+2*sizeof(ptr)+size);
}

list_stack ls_cons(void * thing, ls_uint size, list_stack ls) {
  if (!ls_room_for(size,ls)){
    warn("Data is too large for stack\n");
    return NULL;
  }
  ptr head = ls->head;
  ptr start = head + sizeof(ptr);
  if ((ls_uint)start %ALIGN !=0 ) {
    warn("Warning! Destination address not aligned!\n");
  }
   if ( (ls_uint)thing%ALIGN!=0) {
    warn("Warning! Source address not aligned!\n");
  }
  memcpy(start, thing, size);
  ptr* new_head = (ptr *)((ls_uint)start + (ls_uint)size);
  *new_head = head;
  ls->head = new_head;
  return ls;
}

/* Create a list with an empty data section.  Returns the pointer to the empty data section */
ptr ls_cons_blank(ls_uint size, list_stack ls){
  if (!ls_room_for(size,ls)){
    warn("Data is too large for stack\n");
    return NULL;
  }
  
  if (( ls_uint )size %ALIGN !=0 ) {
    warn("Warning! Size(%d) is not a multiple of alignment(%d)!\n",size, ALIGN);
  }
  ptr head = ls->head;
  ptr start = head + sizeof(ptr);
  if (( ls_uint )start %ALIGN !=0 ) {
    warn("Warning! Destination address not aligned!\n");
  }
  ptr* new_head = (ptr *)((ptr)start + size);
  *new_head = head;
  ls->head = new_head;
  return start;
}

void ls_test() {
  char mem[500];
  list_stack ls   = new_list_stack(mem, 1000);
  const char  hello_world[]  = "Hello World.   ";
  const char  greet[] =        "How are you today? ";

 
  //Serial.printf("Adding first string of length %d, starting at %p\n", strl, hello_world);
  char * target = (char *)ls_cons_blank(strlen(greet)+1, ls);
  strcpy(target, greet);
  int strl = strlen(hello_world) + 1;
  target = (char *)ls_cons_blank(strl, ls);
  strcpy(target, hello_world);

  for (ptr i = ls_start(ls); !ls_is_end(i); i=ls_tail(i)){
    warn("%s",(char *)ls_head(i));
  }
  warn("\n");
}



void setup() {
  // put your setup code here, to run once:
#ifdef ARDUINO
  Serial.begin(9600);
  Serial.print("Starting tests....\n");
  Serial.print("Pointer size is: ");
  Serial.println(sizeof(ptr));
  Serial.print("ls_uint size is: ");
  Serial.println(sizeof(ls_uint));
#endif
  ls_test();
}

void loop() {
  // put your main code here, to run repeatedly:

}

/*
int main() {
	ls_test();
}
*/
