# rorm-pp
C++26 **R**eflection based **O**bject **R**elational **M**apping for C++

## About
The project is a C++26 ORM prototype built around standard reflection.
[The goal/purpose of the project](./doc/management/goal.md)

## Contribution Guidelines
- Please follow the coding style specified in [coding_style.md](./doc/development/coding_style.md).
- Please follow [Contributor Covenant Code of Conduct](https://www.contributor-covenant.org/version/2/1/code_of_conduct/)


## Documentation
### Management
All the management documentation is placed in [doc/management](./doc/management).
- The release scope: [release.md](./doc/management/release-0.1.0/release.md).
- The release roadmap and milestones: [roadmap.md](./doc/management/release-0.1.0/roadmap.md).

### Development
All the development documentation is placed in [doc/development](./doc/development).

### Usage documentation
- TBD

## Licensing
The library is licensed under the Apache License, Version 2.0. Please see [LICENSE](./LICENSE) for more details.

## Dev container
The repository provides a multi-architecture VS Code dev container with GCC 16.1 and the tools needed for development. When the container is
created, its post-create step compiles a standalone C++26 reflection example to verify the toolchain. It does not configure or build rorm.

### Requirements
- Docker Desktop
- Visual Studio Code with the [Dev Containers extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers)

### Getting started
1. Start Docker Desktop.
2. Open the repository in Visual Studio Code.
3. Run **Dev Containers: Reopen in Container** from the command palette.
4. Wait for the post-create reflection smoke compilation to complete.

To repeat the standalone compiler check inside the container, run:

```shell
bash .devcontainer/post-create.sh
```

The container uses the official multi-architecture `gcc:16.1.0` image pinned to an immutable digest. When updating the image digest, rebuild
the container and repeat the post-create validation before committing the update.

## Prerequisites
- `CMake 3.25` or higher
- GCC 16.1 or newer
- Ninja

## Building
Configure and build the project with GCC:

```shell
cmake -S . -B build-debug -G Ninja -DCMAKE_CXX_COMPILER=g++ -DCMAKE_BUILD_TYPE=Debug
cmake --build build-debug --parallel
```


## Testing
Run all registered tests from a configured build directory:

```shell
ctest --test-dir build-debug --output-on-failure
```

## Benchmarking
Benchmarks use [Google Benchmark](https://github.com/google/benchmark) and are disabled by default. Configure a separate Release build and
enable them explicitly:

```shell
cmake -S . -B build-benchmark -G Ninja -DCMAKE_CXX_COMPILER=g++ -DCMAKE_BUILD_TYPE=Release -DRORM_BUILD_BENCHMARKS=ON
cmake --build build-benchmark --target rorm_benchmark --parallel
./build-benchmark/benchmarks/rorm_benchmark
```

Add benchmarks to `benchmarks/benchmark_dummy.cc`, or replace that dummy source when the first real benchmark is introduced.


## Footer
**Founded** in Feb 2025.
