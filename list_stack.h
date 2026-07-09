#ifndef LIST_STACK_H
#define LIST_STACK_H

#include <stddef.h>

typedef unsigned char *ptr;
typedef const unsigned char *const_ptr;
typedef struct ls_header *list_stack;

list_stack new_list_stack(ptr mem, size_t size);
ptr ls_start(list_stack ls);
ptr ls_car(ptr node);
ptr ls_head(ptr node);
ptr ls_cdr(ptr node);
ptr ls_tail(ptr node);
int ls_is_end(ptr node);
size_t ls_data_size(ptr node);
int ls_room_for(size_t size, list_stack ls);
ptr ls_cons_blank(size_t size, list_stack ls);
list_stack ls_cons(const_ptr thing, size_t size, list_stack ls);
int ls_pop(list_stack ls);
void ls_clear(list_stack ls);
size_t ls_alignment(void);
size_t ls_header_size(void);
size_t ls_entry_overhead(void);
size_t ls_align_size(size_t size);
size_t ls_capacity(list_stack ls);
size_t ls_used(list_stack ls);
size_t ls_available(list_stack ls);

#endif
