#include "list_stack.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>

typedef union test_buffer
{
  long double alignment;
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
  ptr one = 0;
  ptr three = 0;
  EXPECT(ls != 0);
  one = ls_cons_blank(1, ls);
  EXPECT(one != 0);
  EXPECT(((uintptr_t)ls_start(ls) % ls_alignment()) == 0);
  EXPECT(((uintptr_t)one % ls_alignment()) == 0);
  three = ls_cons_blank(3, ls);
  EXPECT(three != 0);
  EXPECT(((uintptr_t)ls_start(ls) % ls_alignment()) == 0);
  EXPECT(((uintptr_t)three % ls_alignment()) == 0);
  return 0;
}

static int test_invalid_inputs(void)
{
  test_buffer buffer;
  list_stack ls = fresh_stack(&buffer);
  EXPECT(new_list_stack(0, sizeof(buffer.bytes)) == 0);
  EXPECT(new_list_stack(buffer.bytes, ls_header_size() - 1) == 0);
  EXPECT(new_list_stack(buffer.bytes + 1, sizeof(buffer.bytes) - 1) == 0);
  EXPECT(ls != 0);
  EXPECT(ls_cons(0, 1, ls) == 0);
  EXPECT(ls_cons(0, 0, ls) == ls);
  EXPECT(ls_data_size(ls_start(ls)) == 0);
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
    {"invalid inputs", test_invalid_inputs},
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
