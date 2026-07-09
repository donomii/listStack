# listStack

`listStack` is a small C stack/list that stores variable-size byte entries inside caller-owned memory. It does not allocate or free memory after initialization, so it fits embedded code and fixed-buffer programs.

## Memory Model

Call `new_list_stack(mem, size)` with an aligned memory buffer. The library writes a control header at the start of the buffer, then stores entries sequentially after it.

Each entry contains:

- entry metadata;
- the user payload;
- padding so the next entry remains pointer-aligned.

The newest entry is the list head. `ls_pop` removes the newest entry and makes that space available again. `ls_clear` drops every entry and reuses the whole payload area.

Returned payload pointers point into the caller-owned buffer. They remain valid until the entry is popped, cleared, or overwritten by a later push after popping.

## API

Include `list_stack.h`.

- `new_list_stack(mem, size)` initializes the buffer and returns a stack handle, or `0` when `mem` is null, unaligned, or too small.
- `ls_cons(data, size, ls)` copies `size` bytes into a new entry and returns `ls`, or `0` when the entry cannot fit.
- `ls_cons_blank(size, ls)` creates a new entry and returns its writable payload pointer, or `0` when the entry cannot fit.
- `ls_start(ls)` returns the newest entry node.
- `ls_head(node)` and `ls_car(node)` return the payload pointer for a node.
- `ls_tail(node)` and `ls_cdr(node)` return the next node.
- `ls_is_end(node)` returns true when iteration reached the end.
- `ls_data_size(node)` returns the original payload size for a node.
- `ls_room_for(size, ls)` reports whether one more entry with `size` payload bytes can fit.
- `ls_pop(ls)` removes the newest entry and returns `1`, or returns `0` when the stack is empty or invalid.
- `ls_clear(ls)` removes all entries.
- `ls_alignment()` returns the required memory alignment.
- `ls_header_size()` returns the aligned control-header size.
- `ls_entry_overhead()` returns the aligned per-entry metadata size.
- `ls_align_size(size)` returns the payload size rounded up to stack alignment.
- `ls_capacity(ls)` returns the bytes available for entries after the header.
- `ls_used(ls)` returns entry bytes currently used.
- `ls_available(ls)` returns the largest single payload that can be inserted next.

## Example

```c
#include "list_stack.h"

#include <stdio.h>
#include <string.h>

typedef union buffer
{
  long double alignment;
  unsigned char bytes[1000];
} buffer;

int main(void)
{
  buffer mem;
  list_stack ls = new_list_stack(mem.bytes, sizeof(mem.bytes));
  const char hello_world[] = "Hello World.   ";
  const char greet[] = "How are you today? ";
  char *target = (char *)ls_cons_blank(strlen(greet) + 1, ls);
  strcpy(target, greet);
  ls_cons((const_ptr)hello_world, strlen(hello_world) + 1, ls);
  for (ptr i = ls_start(ls); !ls_is_end(i); i = ls_tail(i)) {
    printf("%s", (const char *)ls_head(i));
  }
  printf("\n");
  return 0;
}
```

## Build And Run

The default settings are:

- `CC=cc`: compiler used by `make`.
- `CFLAGS=-std=c11 -Wall -Wextra -Wpedantic -Werror`: strict C11 warning policy.
- `AR=ar`: static-library archiver.
- `PREFIX=$(CURDIR)/dist`: local install directory used by `make install`.

Commands:

- `./build.sh` builds `liblist_stack.a`, `list_stack_demo`, and `list_stack_tests`.
- `./test.sh` builds and runs the test binary.
- `./demo.sh` builds and runs the demo.
- `./run.sh` is an alias for the demo.
- `./install.sh` installs the header and static library into `dist/`.

## License

This project is licensed under GPL-3.0-only. For broad embedded-library reuse, a permissive license such as MIT, BSD, or Apache-2.0 may be easier for downstream users.
