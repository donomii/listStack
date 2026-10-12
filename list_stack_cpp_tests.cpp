#include "list_stack.h"

#include <cstddef>
#include <cstdio>
#include <cstring>

int main()
{
  alignas(std::max_align_t) unsigned char buffer[512] = {};
  const unsigned char value[] = "C++ caller";
  list_stack ls = new_list_stack(buffer, sizeof(buffer));
  if (ls == 0) {
    std::printf("FAIL C++ caller: could not initialize a 512-byte aligned arena\n");
    return 1;
  } else if (ls_cons(value, sizeof(value), ls) != ls) {
    std::printf("FAIL C++ caller: could not push an %zu-byte payload\n", sizeof(value));
    return 1;
  } else if (ls_data_size(ls_start(ls)) != sizeof(value)) {
    std::printf("FAIL C++ caller: expected %zu payload bytes, received %zu\n",
                sizeof(value), ls_data_size(ls_start(ls)));
    return 1;
  } else if (std::memcmp(ls_head(ls_start(ls)), value, sizeof(value)) != 0) {
    std::printf("FAIL C++ caller: stored payload differs from the original C++ caller string\n");
    return 1;
  } else if (ls_pop(ls) != 1 || !ls_is_end(ls_start(ls)) || ls_used(ls) != 0) {
    std::printf("FAIL C++ caller: popping the only payload did not restore the empty stack\n");
    return 1;
  } else {
    std::printf("PASS C++ caller linked to the C library\n");
    return 0;
  }
}
