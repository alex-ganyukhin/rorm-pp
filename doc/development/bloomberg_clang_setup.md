## Building Bloomberg's Clang with P2996 reflection proposal
- At the moment the library is compatible with Bloomberg's [P2996 reflection proposal implementation](https://github.com/bloomberg/clang-p2996/tree/p2996)
  - Clone the repository
  - Checkout to the `p2996` branch
  - `cd` to the repository
  - Generate the build files with CMake: `cmake -S llvm -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DLLVM_USE_LINKER=lld -DLLVM_ENABLE_PROJECTS="clang;clang-tools-extra" -DLLVM_ENABLE_RUNTIMES="compiler-rt;libcxx;libcxxabi;libunwind" -DLLVM_RUNTIME_TARGETS="x86_64-linux-gnu"`
  - Build `ninja -C build -j$(nproc)`
