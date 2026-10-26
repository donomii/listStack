#include "list_stack.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef union test_buffer
{
  max_align_t alignment;
  unsigned char bytes[512];
} test_buffer;

typedef int (*test_fn)(void);

typedef struct test_case
{
  const char *name;
  test_fn run;
} test_case;

static int fail_check(const char *test_name, int line, const char *condition)
{
  printf("FAIL %s:%d expected %s\n", test_name, line, condition);
  return 1;
}

#define EXPECT(condition) do { if (condition) { } else { return fail_check(__func__, __LINE__, #condition); } } while (0)

static list_stack fresh_stack(test_buffer *buffer)
{
  memset(buffer->bytes, 0xA5, sizeof(buffer->bytes));
  return new_list_stack(buffer->bytes, sizeof(buffer->bytes));
}

static int test_empty_stack(void)
{
  test_buffer buffer;
  list_stack ls = fresh_stack(&buffer);
  EXPECT(ls != 0);
  EXPECT(ls_is_end(ls_start(ls)));
  EXPECT(ls_head(ls_start(ls)) == 0);
  EXPECT(ls_pop(ls) == 0);
  EXPECT(ls_used(ls) == 0);
  EXPECT(ls_capacity(ls) > 0);
  return 0;
}

static int test_cons_and_iteration(void)
{
  test_buffer buffer;
  list_stack ls = fresh_stack(&buffer);
  const char hello_world[] = "Hello World.   ";
  const char greet[] = "How are you today? ";
  char *target = 0;
  ptr node = 0;
  EXPECT(ls != 0);
  target = (char *)ls_cons_blank(strlen(greet) + 1, ls);
  EXPECT(target != 0);
  strcpy(target, greet);
  EXPECT(ls_cons((const_ptr)hello_world, strlen(hello_world) + 1, ls) == ls);
  node = ls_start(ls);
  EXPECT(node != 0);
  EXPECT(strcmp((const char *)ls_head(node), hello_world) == 0);
  EXPECT(ls_data_size(node) == strlen(hello_world) + 1);
  node = ls_tail(node);
  EXPECT(node != 0);
  EXPECT(strcmp((const char *)ls_head(node), greet) == 0);
  EXPECT(ls_data_size(node) == strlen(greet) + 1);
  EXPECT(ls_is_end(ls_tail(node)));
  return 0;
}

static int test_pop_and_reuse(void)
{
  test_buffer buffer;
  list_stack ls = fresh_stack(&buffer);
  const char first[] = "first";
  const char second[] = "second";
  size_t empty_available = 0;
  EXPECT(ls != 0);
  empty_available = ls_available(ls);
  EXPECT(ls_cons((const_ptr)first, strlen(first) + 1, ls) == ls);
  EXPECT(ls_cons((const_ptr)second, strlen(second) + 1, ls) == ls);
  EXPECT(strcmp((const char *)ls_head(ls_start(ls)), second) == 0);
  EXPECT(ls_pop(ls) == 1);
  EXPECT(strcmp((const char *)ls_head(ls_start(ls)), first) == 0);
  EXPECT(ls_pop(ls) == 1);
  EXPECT(ls_pop(ls) == 0);
  EXPECT(ls_is_end(ls_start(ls)));
  EXPECT(ls_available(ls) == empty_available);
  return 0;
}

static int test_clear(void)
{
  test_buffer buffer;
  list_stack ls = fresh_stack(&buffer);
  const char value[] = "value";
  size_t empty_available = 0;
  EXPECT(ls != 0);
  empty_available = ls_available(ls);
  EXPECT(ls_cons((const_ptr)value, strlen(value) + 1, ls) == ls);
  EXPECT(ls_used(ls) > 0);
  ls_clear(ls);
  EXPECT(ls_used(ls) == 0);
  EXPECT(ls_available(ls) == empty_available);
  EXPECT(ls_is_end(ls_start(ls)));
  return 0;
}

static int test_exact_capacity_and_overflow(void)
{
  test_buffer buffer;
  list_stack ls = fresh_stack(&buffer);
  size_t available = 0;
  ptr target = 0;
  EXPECT(ls != 0);
  available = ls_available(ls);
  EXPECT(available > 0);
  target = ls_cons_blank(available, ls);
  EXPECT(target != 0);
  EXPECT(ls_available(ls) == 0);
  EXPECT(ls_room_for(1, ls) == 0);
  EXPECT(ls_cons_blank(1, ls) == 0);
  return 0;
}

static int test_alignment_after_variable_sizes(void)
{
  test_buffer buffer;
  list_stack ls = fresh_stack(&buffer);
  size_t sizes[] = {0, 1, 3, 0, 17, sizeof(max_align_t)};
  size_t i = 0;
  EXPECT(ls != 0);
  EXPECT(ls_alignment() == _Alignof(max_align_t));
  EXPECT(ls_header_size() % _Alignof(max_align_t) == 0);
  EXPECT(ls_entry_overhead() % _Alignof(max_align_t) == 0);
  for (i = 0; i < sizeof(sizes) / sizeof(sizes[0]); i++) {
    ptr target = ls_cons_blank(sizes[i], ls);
    EXPECT(target != 0);
    EXPECT(((uintptr_t)ls_start(ls) % _Alignof(max_align_t)) == 0);
    EXPECT(((uintptr_t)target % _Alignof(max_align_t)) == 0);
    EXPECT(ls_data_size(ls_start(ls)) == sizes[i]);
  }
  return 0;
}

static int test_arena_source_overlap(void)
{
  ptrdiff_t offsets[] = {-(ptrdiff_t)ls_entry_overhead(), -1, 0, 1};
  unsigned char expected[64];
  size_t i = 0;
  size_t j = 0;
  for (i = 0; i < sizeof(expected); i++) {
    expected[i] = (unsigned char)(i + 1);
  }
  for (j = 0; j < sizeof(offsets) / sizeof(offsets[0]); j++) {
    test_buffer buffer;
    list_stack ls = fresh_stack(&buffer);
    ptr target = buffer.bytes + ls_header_size() + ls_entry_overhead();
    ptr source = target + offsets[j];
    EXPECT(ls != 0);
    memcpy(source, expected, sizeof(expected));
    EXPECT(ls_cons(source, sizeof(expected), ls) == ls);
    EXPECT(ls_head(ls_start(ls)) == target);
    EXPECT(memcmp(target, expected, sizeof(expected)) == 0);
    EXPECT(ls_data_size(ls_start(ls)) == sizeof(expected));
    EXPECT(ls_tail(ls_start(ls)) == 0);
    EXPECT(ls_used(ls) == ls_entry_overhead() + ls_align_size(sizeof(expected)));
  }
  return 0;
}

static int test_copy_live_payload(void)
{
  test_buffer buffer;
  list_stack ls = fresh_stack(&buffer);
  const unsigned char value[] = "copy an existing live payload";
  ptr original = 0;
  EXPECT(ls != 0);
  EXPECT(ls_cons(value, sizeof(value), ls) == ls);
  original = ls_start(ls);
  EXPECT(ls_cons(ls_head(original), sizeof(value), ls) == ls);
  EXPECT(ls_start(ls) != original);
  EXPECT(ls_tail(ls_start(ls)) == original);
  EXPECT(memcmp(ls_head(ls_start(ls)), value, sizeof(value)) == 0);
  EXPECT(memcmp(ls_head(original), value, sizeof(value)) == 0);
  return 0;
}

static int test_copy_control_header(void)
{
  test_buffer buffer;
  list_stack ls = fresh_stack(&buffer);
  unsigned char expected[sizeof(buffer.bytes)];
  ptr original = 0;
  EXPECT(ls != 0);
  EXPECT(ls_cons_blank(1, ls) != 0);
  original = ls_start(ls);
  EXPECT(ls_header_size() <= sizeof(expected));
  memcpy(expected, buffer.bytes, ls_header_size());
  EXPECT(ls_cons(buffer.bytes, ls_header_size(), ls) == ls);
  EXPECT(memcmp(ls_head(ls_start(ls)), expected, ls_header_size()) == 0);
  EXPECT(ls_data_size(ls_start(ls)) == ls_header_size());
  EXPECT(ls_tail(ls_start(ls)) == original);
  return 0;
}

static int test_failed_push_preserves_arena(void)
{
  test_buffer buffer;
  list_stack ls = fresh_stack(&buffer);
  const unsigned char value[] = "retained payload";
  unsigned char expected[sizeof(buffer.bytes)];
  ptr original = 0;
  size_t used = 0;
  size_t available = 0;
  size_t largest_aligned = SIZE_MAX - (SIZE_MAX % ls_alignment());
  EXPECT(ls != 0);
  EXPECT(ls_cons(value, sizeof(value), ls) == ls);
  original = ls_start(ls);
  used = ls_used(ls);
  available = ls_available(ls);
  memcpy(expected, buffer.bytes, sizeof(expected));
  EXPECT(ls_cons(0, 1, ls) == 0);
  EXPECT(ls_cons_blank(available + 1, ls) == 0);
  EXPECT(ls_cons(buffer.bytes, available + 1, ls) == 0);
  EXPECT(ls_room_for(SIZE_MAX, ls) == 0);
  EXPECT(ls_cons_blank(SIZE_MAX, ls) == 0);
  EXPECT(ls_cons(buffer.bytes, SIZE_MAX, ls) == 0);
  EXPECT(ls_room_for(largest_aligned, ls) == 0);
  EXPECT(ls_cons_blank(largest_aligned, ls) == 0);
  EXPECT(ls_start(ls) == original);
  EXPECT(ls_used(ls) == used);
  EXPECT(ls_available(ls) == available);
  EXPECT(memcmp(buffer.bytes, expected, sizeof(expected)) == 0);
  return 0;
}

static int test_typed_allocated_payload(void)
{
  ptr memory = (ptr)malloc(512);
  list_stack ls = 0;
  long double *value = 0;
  EXPECT(memory != 0);
  ls = new_list_stack(memory, 512);
  EXPECT(ls != 0);
  EXPECT(ls_cons_blank(1, ls) != 0);
  value = (long double *)ls_cons_blank(sizeof(*value), ls);
  EXPECT(value != 0);
  EXPECT((uintptr_t)value % _Alignof(long double) == 0);
  *value = 1.25L;
  EXPECT(ls_cons((const_ptr)value, sizeof(*value), ls) == ls);
  EXPECT(*(long double *)ls_head(ls_start(ls)) == 1.25L);
  EXPECT(ls_pop(ls) == 1);
  EXPECT(*(long double *)ls_head(ls_start(ls)) == 1.25L);
  free(memory);
  return 0;
}

static int test_invalid_inputs(void)
{
  test_buffer buffer;
  test_buffer offset_buffer;
  list_stack ls = fresh_stack(&buffer);
  list_stack offset_stack = 0;
  memset(offset_buffer.bytes, 0xA5, sizeof(offset_buffer.bytes));
  offset_stack = new_list_stack(offset_buffer.bytes + 1, sizeof(offset_buffer.bytes) - 1);
  EXPECT(new_list_stack(0, sizeof(buffer.bytes)) == 0);
  EXPECT(new_list_stack(buffer.bytes, ls_header_size() - 1) == 0);
  if (ls_alignment() == 1) {
    EXPECT(offset_stack != 0);
    EXPECT(ls_used(offset_stack) == 0);
    EXPECT(ls_is_end(ls_start(offset_stack)));
    EXPECT(ls_capacity(offset_stack) == sizeof(offset_buffer.bytes) - 1 - ls_header_size());
    EXPECT(ls_cons(0, 0, offset_stack) == offset_stack);
    EXPECT(ls_pop(offset_stack) == 1);
  } else {
    EXPECT(offset_stack == 0);
    for (size_t i = 0; i < sizeof(offset_buffer.bytes); i++) {
      EXPECT(offset_buffer.bytes[i] == 0xA5);
    }
  }
  EXPECT(ls != 0);
  EXPECT(ls_cons(0, 1, ls) == 0);
  EXPECT(ls_cons(0, 0, ls) == ls);
  EXPECT(ls_data_size(ls_start(ls)) == 0);
  return 0;
}

static int test_null_handles(void)
{
  const unsigned char value = 1;
  EXPECT(ls_start(0) == 0);
  EXPECT(ls_head(0) == 0);
  EXPECT(ls_car(0) == 0);
  EXPECT(ls_tail(0) == 0);
  EXPECT(ls_cdr(0) == 0);
  EXPECT(ls_is_end(0));
  EXPECT(ls_data_size(0) == 0);
  EXPECT(ls_cons_blank(0, 0) == 0);
  EXPECT(ls_cons_blank(1, 0) == 0);
  EXPECT(ls_cons(0, 0, 0) == 0);
  EXPECT(ls_cons(&value, sizeof(value), 0) == 0);
  EXPECT(ls_pop(0) == 0);
  ls_clear(0);
  EXPECT(ls_room_for(0, 0) == 0);
  EXPECT(ls_room_for(1, 0) == 0);
  EXPECT(ls_capacity(0) == 0);
  EXPECT(ls_used(0) == 0);
  EXPECT(ls_available(0) == 0);
  return 0;
}

int main(void)
{
  test_case tests[] = {
    {"empty stack", test_empty_stack},
    {"cons and iteration", test_cons_and_iteration},
    {"pop and reuse", test_pop_and_reuse},
    {"clear", test_clear},
    {"exact capacity and overflow", test_exact_capacity_and_overflow},
    {"alignment after variable sizes", test_alignment_after_variable_sizes},
    {"arena source overlap", test_arena_source_overlap},
    {"copy live payload", test_copy_live_payload},
    {"copy control header", test_copy_control_header},
    {"failed push preserves arena", test_failed_push_preserves_arena},
    {"typed allocated payload", test_typed_allocated_payload},
    {"invalid inputs", test_invalid_inputs},
    {"null handles", test_null_handles},
  };
  size_t i = 0;
  for (i = 0; i < sizeof(tests) / sizeof(tests[0]); i++) {
    if (tests[i].run() == 0) {
      printf("PASS %s\n", tests[i].name);
    } else {
      return 1;
    }
  }
  printf("%zu tests passed\n", sizeof(tests) / sizeof(tests[0]));
  return 0;
}
