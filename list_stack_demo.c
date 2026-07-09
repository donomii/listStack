#include "list_stack.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#ifdef ARDUINO
#include <Arduino.h>
#endif

typedef union demo_buffer
{
  long double alignment;
  unsigned char bytes[1000];
} demo_buffer;

static void demo_print(const char *text)
{
#ifdef ARDUINO
  Serial.print(text);
#else
  printf("%s", text);
#endif
}

int list_stack_demo_run(void)
{
  demo_buffer buffer;
  list_stack ls = new_list_stack(buffer.bytes, sizeof(buffer.bytes));
  const char hello_world[] = "Hello World.   ";
  const char greet[] = "How are you today? ";
  char *target = 0;
  ptr i = 0;
  if (ls == 0) {
    demo_print("new_list_stack failed: expected aligned memory with room for the header\n");
    return 1;
  } else {
    target = (char *)ls_cons_blank(strlen(greet) + 1, ls);
    if (target == 0) {
      demo_print("ls_cons_blank failed: expected room for greeting text\n");
      return 1;
    } else {
      strcpy(target, greet);
      if (ls_cons((const_ptr)hello_world, strlen(hello_world) + 1, ls) == 0) {
        demo_print("ls_cons failed: expected room for hello text\n");
        return 1;
      } else {
        for (i = ls_start(ls); !ls_is_end(i); i = ls_tail(i)) {
          demo_print((const char *)ls_head(i));
        }
        demo_print("\n");
        return 0;
      }
    }
  }
}

#ifdef ARDUINO
void setup(void)
{
  Serial.begin(9600);
  list_stack_demo_run();
}

void loop(void)
{
}
#else
int main(void)
{
  return list_stack_demo_run();
}
#endif
