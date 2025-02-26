# rorm-pp
C++26 **R**eflection based **O**bject **R**elational **M**apping for C++

## About
As it can be understood from the name, the library provides ORM capabilities based purely on C++26 built-in reflection.


## Contribution Guidelines
- Please follow the coding style specified in [coding_style.md](./doc/development/coding_style.md).
- Please follow [Contributor Covenant Code of Conduct](https://www.contributor-covenant.org/version/2/1/code_of_conduct/)


## Documentation
### Management
All the management documentation is placed in [doc/management](./doc/management).
- The full roadmap: [roadmap.md](./doc/management/roadmap.md).

### Development
All the development documentation is placed in [doc/development](./doc/development).
- [Coding style](./doc/development/coding_style.md)
- [C++26 reflection](./doc/development/cpp_reflection.md)

### Usage documentation
- TBD


## Licensing
The library is licensed under the Apache License, Version 2.0. Please see [LICENSE](./LICENSE) for more details.

## Prerequisites
- `CMake 3.25` or higher
- Compiler supporting P2996 reflection proposal
  - **Warning**: ATM only [Bloomberg's Clang](#building-bloombergs-clang-with-p2996-reflection-proposal) is supported.

### Building Bloomberg's Clang with P2996 reflection proposal
- At the moment the library is compatible with Bloomberg's [P2996 reflection proposal implementation](https://github.com/bloomberg/clang-p2996/tree/p2996)
  - Clone the repository
  - Checkout to the `p2996` branch
  - `cd` to the repository
  - Generate the build files with CMake: `cmake -S llvm -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DLLVM_USE_LINKER=lld -DLLVM_ENABLE_PROJECTS="clang;clang-tools-extra" -DLLVM_ENABLE_RUNTIMES="compiler-rt;libcxx;libcxxabi;libunwind" -DLLVM_RUNTIME_TARGETS="x86_64-linux-gnu"`
  - Build `ninja -C build -j$(nproc)`

## Building
1. **Precondition**: [Prerequisites](#prerequisites) are installed.
2. `mkdir build-debug && cd build-debug`
3. `cmake -G Ninja -DCOMPILER_DIR=/abs/path/to/bloomberg/clang/build -DCMAKE_BUILD_TYPE=Debug ..`
4. `ninja`
5. `LD_LIBRARY_PATH=/abs/path/to/bloomberg/clang/build/lib/x86_64-unknown-linux-gnu ./rorm-test`


## Testing
- TBD


## Footer
**Founded** in Feb 2025.
