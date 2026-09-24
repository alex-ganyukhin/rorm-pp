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

The [project constitution](../../.specify/memory/constitution.md) is authoritative for
quality gates, tests, API documentation, and performance policy. Add focused guides only
where API comments and tests do not provide an adequate learning path.
