# rorm-pp goals

rorm-pp aims to be an adoptable C++26 ORM that uses standard reflection to map ordinary C++ types with minimal boilerplate.

## Product principles

- **Standard reflection**: derive mappings from C++26 reflection instead of macros, generated registration code, or parallel schemas.
- **Compile-time safety**: reject invalid C++ mappings, unsupported values, and incompatible typed operations during compilation where the required information is available.
- **Value-oriented safety**: prefer direct value types and avoid pointers, ownership machinery, and runtime polymorphism unless their semantics require them.
- **Simple records**: support public data fields and explicitly marked, conventional getter/setter properties.
- **Focused customization**: allow mappings to override names, identify keys and generated values, exclude properties, and convert custom value types.
- **Escape hatches**: provide parameterized raw SQL for operations outside the typed ORM feature set.
- **Portability**: keep the product design cross-platform even when reflection toolchain availability limits the platforms that can be validated.
- **Quality as documentation**: keep public APIs documented with Doxygen and behavior demonstrated by comprehensive unit and subsystem tests.
  - See more in the [Quality objectives](./goal.md#quality-objectives).
- **Measured performance**: maintain repeatable benchmarks before making performance claims.

## Quality objectives

Each contribution to the project must meet the following quality objectives:

- at least 95% Doxygen coverage of its public APIs;
- at least 95% line coverage and 95% branch coverage;
- unit and subsystem tests for its successful behavior and relevant failures; and
- clean `-Wall -Wextra -Werror` builds under GCC 16.1 or newer.

Tests and Doxygen comments are the primary product documentation. Add focused guides only where they provide a learning path that API comments and tests cannot.


## Performance objectives

- Maintain repeatable benchmarks from release `0.1.0` onward, but make no competitive performance claim until a specific target is adopted.
- In the initial releases, the performance objective is not so critical. Execution paths may be suboptimal, but they must be correct and safe. The performance objective will become more important as the product matures.
