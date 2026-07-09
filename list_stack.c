#include "list_stack.h"

#include <stdint.h>
#include <string.h>

#define LS_ALIGNMENT sizeof(uintptr_t)

struct ls_header
{
  size_t size;
  size_t used;
  ptr head;
};

struct ls_node
{
  ptr previous;
  size_t size;
};

static int ls_align_checked(size_t size, size_t *aligned)
{
  size_t remainder = size % LS_ALIGNMENT;
  if (remainder == 0) {
    *aligned = size;
    return 1;
  } else {
    size_t padding = LS_ALIGNMENT - remainder;
    if (size > SIZE_MAX - padding) {
      return 0;
    } else {
      *aligned = size + padding;
      return 1;
    }
  }
}

static size_t ls_align_down(size_t size)
{
  return size - (size % LS_ALIGNMENT);
}

static size_t ls_node_header_size(void)
{
  size_t aligned = 0;
  if (ls_align_checked(sizeof(struct ls_node), &aligned)) {
    return aligned;
  } else {
    return 0;
  }
}

static ptr ls_data_start(list_stack ls)
{
  return (ptr)ls + ls_header_size();
}

static int ls_has_valid_bounds(list_stack ls)
{
  return ls != 0 && ls->size >= ls_header_size() && ls->used <= ls->size - ls_header_size();
}

size_t ls_alignment(void)
{
  return LS_ALIGNMENT;
}

size_t ls_header_size(void)
{
  size_t aligned = 0;
  if (ls_align_checked(sizeof(struct ls_header), &aligned)) {
    return aligned;
  } else {
    return 0;
  }
}

size_t ls_entry_overhead(void)
{
  return ls_node_header_size();
}

size_t ls_align_size(size_t size)
{
  size_t aligned = 0;
  if (ls_align_checked(size, &aligned)) {
    return aligned;
  } else {
    return 0;
  }
}

list_stack new_list_stack(ptr mem, size_t size)
{
  if (mem == 0) {
    return 0;
  } else if (((uintptr_t)mem % LS_ALIGNMENT) != 0) {
    return 0;
  } else if (size < ls_header_size()) {
    return 0;
  } else {
    list_stack ls = (list_stack)mem;
    ls->size = size;
    ls->used = 0;
    ls->head = 0;
    return ls;
  }
}

ptr ls_start(list_stack ls)
{
  if (ls == 0) {
    return 0;
  } else {
    return ls->head;
  }
}

ptr ls_car(ptr node)
{
  if (node == 0) {
    return 0;
  } else {
    return node + ls_node_header_size();
  }
}

ptr ls_head(ptr node)
{
  return ls_car(node);
}

ptr ls_cdr(ptr node)
{
  if (node == 0) {
    return 0;
  } else {
    struct ls_node *entry = (struct ls_node *)node;
    return entry->previous;
  }
}

ptr ls_tail(ptr node)
{
  return ls_cdr(node);
}

int ls_is_end(ptr node)
{
  return node == 0;
}

size_t ls_data_size(ptr node)
{
  if (node == 0) {
    return 0;
  } else {
    struct ls_node *entry = (struct ls_node *)node;
    return entry->size;
  }
}

int ls_room_for(size_t size, list_stack ls)
{
  size_t aligned = 0;
  size_t entry_size = 0;
  size_t remaining = 0;
  if (!ls_has_valid_bounds(ls)) {
    return 0;
  } else if (!ls_align_checked(size, &aligned)) {
    return 0;
  } else if (aligned > SIZE_MAX - ls_node_header_size()) {
    return 0;
  } else {
    entry_size = ls_node_header_size() + aligned;
    remaining = ls->size - ls_header_size() - ls->used;
    return entry_size <= remaining;
  }
}

ptr ls_cons_blank(size_t size, list_stack ls)
{
  size_t aligned = 0;
  size_t entry_size = 0;
  ptr node = 0;
  struct ls_node *entry = 0;
  if (!ls_room_for(size, ls)) {
    return 0;
  } else if (!ls_align_checked(size, &aligned)) {
    return 0;
  } else {
    entry_size = ls_node_header_size() + aligned;
    node = ls_data_start(ls) + ls->used;
    entry = (struct ls_node *)node;
    entry->previous = ls->head;
    entry->size = size;
    ls->used += entry_size;
    ls->head = node;
    return node + ls_node_header_size();
  }
}

list_stack ls_cons(const_ptr thing, size_t size, list_stack ls)
{
  ptr target = 0;
  if (thing == 0 && size != 0) {
    return 0;
  } else {
    target = ls_cons_blank(size, ls);
    if (target == 0) {
      return 0;
    } else {
      if (size != 0) {
        memcpy(target, thing, size);
      }
      return ls;
    }
  }
}

int ls_pop(list_stack ls)
{
  ptr node = 0;
  struct ls_node *entry = 0;
  if (!ls_has_valid_bounds(ls)) {
    return 0;
  } else if (ls->head == 0) {
    return 0;
  } else {
    node = ls->head;
    entry = (struct ls_node *)node;
    ls->head = entry->previous;
    ls->used = (size_t)(node - ls_data_start(ls));
    return 1;
  }
}

void ls_clear(list_stack ls)
{
  if (ls == 0) {
    return;
  } else {
    ls->used = 0;
    ls->head = 0;
  }
}

size_t ls_capacity(list_stack ls)
{
  if (ls == 0 || ls->size < ls_header_size()) {
    return 0;
  } else {
    return ls->size - ls_header_size();
  }
}

size_t ls_used(list_stack ls)
{
  if (!ls_has_valid_bounds(ls)) {
    return 0;
  } else {
    return ls->used;
  }
}

size_t ls_available(list_stack ls)
{
  size_t remaining = 0;
  size_t overhead = ls_node_header_size();
  if (!ls_has_valid_bounds(ls)) {
    return 0;
  } else {
    remaining = ls->size - ls_header_size() - ls->used;
    if (remaining < overhead) {
      return 0;
    } else {
      return ls_align_down(remaining - overhead);
    }
  }
}
