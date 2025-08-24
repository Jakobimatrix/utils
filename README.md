# utils

Reusable C++ utility library for cross-project development.

[![C/C++ CI](https://github.com/Jakobimatrix/utils/actions/workflows/ubuntu_build_test.yml/badge.svg)](https://github.com/Jakobimatrix/utils/actions/workflows/ubuntu_build_test.yml)

 - OS: Ubuntu 24.04
 - compiler: clang 19, gcc 13
 - debug + release
 - tests

 ---

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
./initRepo/scripts/build.sh -d -T    # Debug build with tests
./initRepo/scripts/build.sh -r       # Release build
./initRepo/scripts/build.sh -r -f --compiler clang  # Enable fuzzing (requires clang), builds fuzzer targets
```

## Usage

Include headers from `include/utils/` in your project. Link against the built static library. 

**utils_lib_2.0.0**




