# listStack

`listStack` is a small C stack that keeps an entire linked list inside the stack.  It is in effect an arena allocator for linked lists. It's handy for embedded work as it doesn't allocate memory, it works entire within a pre-allocated arena.



## API

Include `list_stack.h`.

- `new_list_stack(mem, size) -> handle`: initialize `mem` as a list stack. Returns `0` when `mem` is null, unaligned, or too small.
- `ls_cons(payload, size, handle) -> handle`: push a copy of `payload`. Returns `0` when `payload` is null with nonzero `size`, or when the node cannot fit.
- `ls_cons_blank(size, handle) -> payload`: push an uninitialized node and return its writable payload. Returns `0` when the node cannot fit.
- `ls_start(handle) -> node`: return the newest node. Returns `0` when the handle has no nodes.
- `ls_head(node) -> payload`: return the node payload. Returns `0` when `node` is `0`.
- `ls_car(node) -> payload`: same as `ls_head`.
- `ls_tail(node) -> node`: return the next node. Returns `0` at the end.
- `ls_cdr(node) -> node`: same as `ls_tail`.
- `ls_is_end(node) -> int`: return `1` when `node` is `0`, otherwise `0`.
- `ls_data_size(node) -> size`: return the node payload size. Returns `0` when `node` is `0`.
- `ls_room_for(size, handle) -> int`: return `1` when a node with `size` payload bytes can fit, otherwise `0`.
- `ls_pop(handle) -> int`: remove the newest node. Returns `1` when a node was removed, otherwise `0`.
- `ls_clear(handle) -> void`: remove all nodes from the handle.
- `ls_alignment() -> size`: return the required buffer and node alignment.
- `ls_header_size() -> size`: return the aligned handle header size.
- `ls_entry_overhead() -> size`: return the aligned per-node metadata size.
- `ls_align_size(size) -> size`: return `size` rounded up to stack alignment.
- `ls_capacity(handle) -> size`: return the byte capacity available for nodes after the handle header.
- `ls_used(handle) -> size`: return the bytes currently used by nodes.
- `ls_available(handle) -> size`: return the largest payload size that can fit in one new node.

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

## Memory Model

Call `new_list_stack(mem, size)` with an aligned memory buffer. The library writes a control header at the start of the buffer, then stores entries sequentially after it.

Each entry contains:

- entry metadata;
- the user payload;
- padding so the next entry remains pointer-aligned.

The newest entry is the list head. `ls_pop` removes the newest entry and makes that space available again. `ls_clear` drops every entry and reuses the whole payload area.

Returned payload pointers point into the caller-owned buffer. They remain valid until the entry is popped, cleared, or overwritten by a later push after popping.

## License

This project is licensed under GPL-3.0 or later.
