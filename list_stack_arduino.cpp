#ifdef ARDUINO
#include <Arduino.h>

#include "list_stack_demo.h"

void demo_print(const char *text)
{
  Serial.print(text);
}

void setup(void)
{
  Serial.begin(9600);
  list_stack_demo_run();
}

void loop(void)
{
}
#endif
