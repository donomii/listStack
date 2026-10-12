# listStack behavior

listStack is a last-in, first-out list stored entirely in caller-supplied memory. Entries have variable-length byte payloads. The library does not allocate or free memory.

## Storage and ownership

The caller supplies a writable buffer, its byte length, and its lifetime. Its address must satisfy `ls_alignment()`, which is the alignment of `max_align_t`. In C11, `_Alignas(max_align_t) unsigned char bytes[512]` provides a suitable buffer; in C++11 use `alignas(max_align_t) unsigned char bytes[512]`.

The buffer contains an aligned control header followed by contiguous entries. Each entry contains aligned metadata, its payload, and padding. Both metadata and payload addresses satisfy `ls_alignment()`, including after zero-length or odd-length payloads. Types with stricter, extended alignment are not supported.

Control and entry metadata are private byte representations. The library must support declared byte arrays without accessing their contents through incompatible structure types. The representation is process-local, contains pointers, and is not a portable file format or a relocatable image.

Payload alignment does not change C's object-type rules. With a declared byte array, transfer typed values using byte copies to and from properly typed variables. Storage obtained from an allocator may instead support direct typed payload access under the language's object-type rules. The caller owns payload initialization and interpretation.

The caller must keep the buffer alive and stationary while using its handle or nodes. Payload and node pointers remain valid until their entry is popped or cleared. Concurrent access requires caller-provided synchronization. Handles and nodes must come from this library and refer to live, uncorrupted storage; null behavior is specified below.

## Public values and operations

- `ptr` and `const_ptr` are mutable and read-only byte pointers. A node pointer refers to entry metadata; a payload pointer refers to user bytes. They are not interchangeable.
- `list_stack` is an opaque handle to a buffer's control header.
- `new_list_stack(buffer, size)` initializes an empty stack and returns its handle. Null, insufficiently aligned, or shorter-than-header buffers return null without modification. Reinitializing a buffer discards its former entries.
- `ls_cons(source, size, stack)` pushes a copy of exactly `size` source bytes and returns the same handle. The source may be anywhere within readable storage, including the stack's control header, live payloads, unused arena bytes, or overlapping destination bytes. The copied bytes are those present before the push changes metadata. A null source is allowed only for zero bytes.
- `ls_cons_blank(size, stack)` pushes an uninitialized payload and returns its writable address. Zero-length entries are allowed and still consume entry metadata; their payload address must not be dereferenced.
- Both push operations return null when the entry cannot fit, the size arithmetic would overflow, or the handle is null. Failure leaves the arena unchanged.
- `ls_start(stack)` returns its newest node, or null for an empty or null stack.
- `ls_head(node)` and `ls_car(node)` return the payload address, or null for a null node.
- `ls_tail(node)` and `ls_cdr(node)` return the preceding, older node, or null at the end or for a null node.
- `ls_is_end(node)` returns whether the node is null.
- `ls_data_size(node)` returns the original payload byte count, excluding padding; a null node returns zero.
- `ls_pop(stack)` removes the newest entry and reclaims its metadata, payload, and padding. It returns one on removal and zero for an empty or null stack.
- `ls_clear(stack)` removes every entry and restores the initial capacity for entries. A null handle is a no-op. Removed bytes need not be erased.
- `ls_alignment()` returns the required buffer, node, and payload alignment in bytes.
- `ls_header_size()` returns the aligned control-header byte count.
- `ls_entry_overhead()` returns the aligned metadata byte count for one entry.
- `ls_align_size(size)` rounds a byte count up to the required alignment; overflow returns zero, as does an input of zero.
- `ls_capacity(stack)` returns buffer bytes remaining after the control header, including space needed for entry metadata and padding.
- `ls_used(stack)` returns bytes consumed by entries, including metadata and padding.
- `ls_available(stack)` returns the largest payload byte count that can fit in one additional entry. Zero alone does not establish whether an empty entry can fit.
- `ls_room_for(size, stack)` returns whether an entry of that payload size can fit, including its metadata and padding. Use this operation to distinguish capacity for zero-length entries.
- Capacity queries return zero for a null stack.

The public header is usable from C and C++. C++ declarations must link to the symbols emitted when the implementation is compiled as C.

## Push behavior

```text
Validate the source requirement and available capacity without changing the arena.
Save the current control metadata.
Determine the new node and payload addresses.
For a copying push, preserve the source bytes in the payload, allowing overlap.
Record the new entry's previous-node pointer and original payload size.
Advance used bytes and make the new node the head.
Return the payload for a blank push, or the stack handle for a copying push.
```

## Build and verification

`./build.sh` builds the C library, demo, and C tests. `./test.sh` runs the C regressions and a C++ caller linked to the C library. `./run.sh` and `./demo.sh` run the demo. `./install.sh` installs the header and library under the local `dist/` directory by default.

Make action targets always execute when requested and do not generate executables named after the actions. `CC` and `CXX` select the C and C++ compilers. `CFLAGS` defaults to strict C11 warnings, and `CXXFLAGS` defaults to `-std=c++11 -Wall -Wextra -Wpedantic -Werror`; override them as make variables to change compilation settings. `AR` selects the archiver and `PREFIX` selects the installation directory.

Regression coverage includes overlapping arena sources, unchanged bytes after rejected pushes, zero-length and variable-length alignment, typed allocator-backed payloads, and C++ linkage. CI runs GCC and Clang, including optimized address and undefined-behavior sanitizer builds.
