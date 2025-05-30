# utils

Reusable C++ utility library for cross-project development.

## Features

- String manipulation (search and replace)
- Filesystem helpers
- Type demangling and debug utilities
- Bitwise enum operations
- Type container and template metaprogramming helpers
- Binary data interpretation
- Cross-platform memory usage queries

## Build

```bash
./build.sh -d -t      # Debug build with tests
./build.sh -r         # Release build
./build.sh -r -f --compiler clang  # Enable fuzzing (requires clang), builds fuzzer targets
```

## Usage

Include headers from `include/utils/` in your project. Link against the built static library. 

**utils_lib_1.0.0**

## Tests

Run with:

```bash
./build-*/test_searchAndReplace
./build-*/test_BinaryDataInterpreter
```


