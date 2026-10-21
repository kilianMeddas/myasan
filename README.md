# MyASan

MyASan is an educational **C99** shared library inspired by AddressSanitizer, developed as part of my studies at **EPITA**. It intercepts heap allocations and deallocations, tracks their lifetime, and uses shadow memory to check explicitly validated memory ranges.

The project combines dynamic linking, allocation tracking, redzones, memory poisoning, and diagnostic reports.

---

## Table of Contents

- [Features](#features)
- [Implementation Roadmap](#implementation-roadmap)
- [Requirements](#requirements)
- [Building](#building)
- [Usage](#usage)
- [Architecture](#architecture)
- [How It Works](#how-it-works)
  - [Allocation Tracking](#allocation-tracking)
  - [Shadow Memory](#shadow-memory)
  - [Diagnostics and Exit Codes](#diagnostics-and-exit-codes)
- [Tests and Maintenance](#tests-and-maintenance)
- [Current Limitations](#current-limitations)

---

## Features

- Intercept `malloc` and `free` through a shared library loaded with `LD_PRELOAD`.
- Track allocation sizes and lifetime in a chained hash map.
- Reserve redzones around allocations and mark their shadow bytes as inaccessible.
- Detect invalid frees and double frees.
- Detect heap buffer overflows and use-after-free through explicit range checks.
- Print an error report, one stack frame outside the library, and a shadow-memory dump for failed range checks.
- Report outstanding allocations and their total size at normal program exit.

---

## Implementation Roadmap

- [x] **`malloc` / `free`** — Track heap allocations and deallocations.
- [x] **Shadow memory** — Mark accessible memory, redzones, and freed regions.
- [x] **Memory checks** — Validate ranges and report detected errors.
- [x] **Leak reports** — List allocations still alive at program exit.
- [ ] **`memcpy`** — Validate source and destination ranges and detect overlap.
- [ ] **`calloc` / `realloc`** — Extend allocation interception.

Unchecked items are proposed next steps, not features available in the current version.

---

## Requirements

- A 64-bit Linux environment with glibc and support for the mapping flags used by the project.
- GCC and GNU Make.
- A dynamically linked target program for `LD_PRELOAD` interposition.
- `clang-format` for the optional formatting target.

The implementation reserves a fixed **16 TiB virtual address range** for shadow memory. This does not allocate 16 TiB of physical RAM, but the requested address range must be available.

---

## Building

From the GitHub repository root:

```sh
git clone https://github.com/kilianMeddas/myasan.git
cd myasan/myasan
make
```

This produces `libmyasan.so` in the directory containing the Makefile.

---

## Usage

### Load the Library

From the build directory, preload the library when launching a dynamically linked program:

```sh
LD_PRELOAD="$PWD/libmyasan.so" ./program
```

Allocation tracking, invalid-free detection, double-free detection, and exit-time leak reporting use the intercepted allocation functions.

### Check a Memory Range

Buffer-overflow and use-after-free detection require an explicit call to `check_ptr_validity`. Ordinary pointer reads and writes are not automatically instrumented.

Save this example as `example.c` in the build directory:

```c
#include <stdlib.h>

#include "poison.h"

int main(void)
{
    char *buffer = malloc(8);
    if (buffer == NULL)
        return 10;

    check_ptr_validity(buffer, 8);  /* Valid range. */
    check_ptr_validity(buffer, 9);  /* Reports heap-buffer-overflow and exits. */

    free(buffer);
    return 0;
}
```

Build and run it:

```sh
gcc -std=c99 -O0 -fno-builtin -g -Iinclude example.c \
    -L"$PWD" -lmyasan -Wl,-rpath,"$PWD" -o example
LD_PRELOAD="$PWD/libmyasan.so" ./example
echo $?  # 3: heap-buffer-overflow
```

The final `free` is not reached after the error. MyASan suppresses the leak report when a memory error has already been reported.

To exercise use-after-free detection instead, replace the two check calls and final `free` with:

```c
free(buffer);
check_ptr_validity(buffer, 1);  /* Reports heap-use-after-free; exits with 2. */
```

### Inspect a Report

A detected memory error produces a header similar to:

```text
=================================================================
==0003== ERROR: AddressSanitizer: heap-buffer-overflow on address ...
```

Failed range checks also print nine lines of shadow bytes, highlighting the offending byte, followed by one stack frame outside the library. Invalid-free and double-free reports include the header and stack frame without a shadow dump.

---

## Architecture

Paths below are relative to the GitHub repository root.

| File or directory | Responsibility |
| --- | --- |
| `myasan/Makefile` | Builds the shared library and standalone test programs; checks their exit codes. |
| `myasan/include/` | Public declarations, shared state, error codes, and memory-layout constants. |
| `myasan/src/myasan.c` | Startup and shutdown hooks, symbol resolution, shadow mapping, and leak-report triggering. |
| `myasan/src/malloc.c` | Allocates redzones and user memory, records allocations, and initializes shadow bytes. |
| `myasan/src/free.c` | Validates pointers, detects repeated frees, poisons shadow memory, and releases the underlying allocation. |
| `myasan/src/hash_map.c` | Stores allocation metadata in a chained hash map. |
| `myasan/src/poison.c` | Updates shadow bytes, checks ranges, and displays shadow-memory diagnostics. |
| `myasan/src/print.c` | Formats memory errors, selects a stack frame, and prints leak summaries. |
| `myasan/tests/malloc/` | Allocation behavior, counters, redzones, and a deliberate leak. |
| `myasan/tests/free/` | Deallocation behavior, counters, null pointers, and freed-memory poisoning. |
| `myasan/tests/hash_map/` | Metadata operations, pointer reuse, and hash collisions. |
| `myasan/tests/poison/` | Valid and invalid shadow states, partial granules, and range boundaries. |
| `myasan/tests/errors/` | Separate processes expected to terminate with specific memory-error codes. |

---

## How It Works

### Allocation Tracking

At startup, the constructor resolves the underlying allocator functions using `dlsym(RTLD_NEXT, ...)`, initializes a hash map, and maps shadow memory.

For each `malloc(size)` call, MyASan:

1. Allocates the requested size plus two **512-byte redzones**.
2. Returns a pointer after the left redzone.
3. Records the user pointer, requested size, and allocation state.
4. Marks the user range as accessible and the redzones as poisoned in shadow memory.

`free` looks up the exact user pointer. Unknown pointers and previously freed pointers produce errors. Valid deallocations poison the user range, update the counters, and pass the original base pointer to the underlying allocator. `free(NULL)` returns immediately.

### Shadow Memory

One shadow byte represents eight bytes of application memory:

```c
shadow_address = 0x100000000000 + (application_address >> 3);
```

| Shadow value | Meaning |
| --- | --- |
| `0x00` | All eight bytes are accessible. |
| `0x01`–`0x07` | Only the first 1–7 bytes are accessible. |
| `0xFA` | Redzone: access is classified as heap-buffer-overflow. |
| `0xFD` | Freed memory: access is classified as heap-use-after-free. |
| Other values | Classified as invalid pointer by the range checker. |

`poison_is_poisoned` inspects a range and returns an error code. `check_ptr_validity` uses that result to print diagnostics and terminate on an error.

### Diagnostics and Exit Codes

| Code | Meaning |
| --- | --- |
| `0` | No detected memory error; leaks alone do not change the program's exit status. |
| `1` | Invalid pointer. |
| `2` | Heap use-after-free. |
| `3` | Heap buffer overflow. |
| `4` | Double free. |
| `5` | Reserved for `memcpy` parameter overlap; detection is not implemented yet. |

These are MyASan's memory-error codes; the target program can also return its own exit status.

The backtrace code compares loaded-object base addresses with `dladdr` to skip frames from MyASan. It does not depend on the shared library's filename.

At normal exit, if the allocation and free counters differ, MyASan lists live allocations and prints a summary:

```text
LeakSanitizer: Detected memory leaks
    -> Leak at ... (size 16 bytes)
SUMMARY: LeakSanitizer: 16 byte(s) leaked in 1 allocation(s).
```

---

## Tests and Maintenance

From the directory containing the Makefile:

```sh
make check    # Build and run all standalone test programs
make clean    # Remove the shared library, object files, and test binaries
make format   # Format source and header files with clang-format
```

The suite contains **28 test programs**. The test runner uses `LD_PRELOAD` and compares each program's exit code with its expected result. Deliberate errors under `tests/errors/` are expected to exit with codes 1–4; other tests are expected to return 0. A deliberate leak may print a report while still passing.

To run one error test:

```sh
make tests/errors/test_double_free
LD_PRELOAD="$PWD/libmyasan.so" ./tests/errors/test_double_free
echo $?  # 4
```

---

## Current Limitations

- Memory accesses are checked only when explicitly passed to the range-checking functions. Arbitrary reads and writes are not automatically monitored.
- `calloc`, `realloc`, and `memcpy` are not intercepted. Resolving a libc symbol during startup does not implement a wrapper for it.
- The fixed shadow layout assumes a compatible 64-bit Linux address space. Startup aborts if the mapping fails.
- Allocation metadata and counters are shared without synchronization; concurrent allocator calls are not protected.
- Freed blocks are returned immediately to the underlying allocator. There is no quarantine, so address reuse limits detection of stale pointers.
- Shadow bytes initialized to zero do not by themselves prove that an address belongs to a tracked allocation. The range checker is not a general pointer-ownership validator.
- Startup reentrancy and calls made before initialization are not fully guarded.

MyASan is a learning project focused on heap diagnostics and allocator interposition, rather than a complete replacement for compiler-instrumented AddressSanitizer.
