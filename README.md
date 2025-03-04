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

### Usage documentation
- TBD

## Licensing
The library is licensed under the Apache License, Version 2.0. Please see [LICENSE](./LICENSE) for more details.

## Prerequisites
- `CMake 3.25` or higher
- Compiler, supporting P2996 reflection proposal
  - **Warning**: ATM only [Bloomberg's Clang](./doc/development/bloomberg_clang_setup.md) is supported.

## Building
1. **Precondition**: [Prerequisites](#prerequisites) are installed.
2. `mkdir build-debug && cd build-debug`
3. `cmake -G Ninja -DCOMPILER_DIR=/abs/path/to/bloomberg/clang/build -DCMAKE_BUILD_TYPE=Debug ..`
4. `ninja`
5. `./tests/rorm_test`


## Testing
- TBD


## Footer
**Founded** in Feb 2025.
