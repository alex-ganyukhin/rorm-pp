# Project roadmap

This roadmap orders work by capability rather than by date or estimated effort. It describes the scope of the current target, release `0.1.0`; future ideas remain intentionally unordered until they become current work.

The [2025 roadmap and milestones](./archive/2025/roadmap.md) are retained as project history.

## Release 0.1.0

`0.1.0` is an early-adopter release for Linux with GCC 16.1 or newer. The product must remain portable by design; macOS development is supported through the Linux development container, while native macOS and Windows validation depend on suitable standard-reflection toolchains.

### Included capabilities

- Records based on public fields or explicitly marked simple getter/setter properties, including direction-specific properties.
- Essential mapping overrides for names, keys, generated values, exclusions, and custom value conversions.
- JSON interchange for Records, nested reflected values, optional values, and common sequences.
- Synchronous PostgreSQL access for existing schemas.
- Typed, parameterized raw SQL and explicit transactions.
- Individual and batch typed CRUD with returned database-generated values.
- Single-table typed filtering, sorting, limit/offset pagination, projections, count, and exists.
- Practical scalar, string, byte, enum, optional/null, and date/time value mappings. PostgreSQL UUID values use `std::string`.
- An installable and consumable CMake package.

### Explicit exclusions

- Relationships and joins.
- Schema creation and migrations.
- Async execution.
- Database backends other than PostgreSQL.
- External mapping of complete, unmodifiable third-party Record types.
- First-class persistence for PostgreSQL JSON and JSONB columns.
- Native-platform guarantees beyond the validated Linux toolchain.
- Publication to C++ package registries.

### Milestones

Every milestone is subject to the [project quality objectives](./goal.md#quality-objectives).

- [ ] [000: Project revival](./milestones/000_project_revival.md)
- [ ] [001: Record foundation](./milestones/001_record_foundation.md)
- [ ] [002: JSON interchange](./milestones/002_json_interchange.md)
- [ ] [003: PostgreSQL execution](./milestones/003_postgresql_execution.md)
- [ ] [004: Typed ORM](./milestones/004_typed_orm.md)
- [ ] [005: Release 0.1.0](./milestones/005_release_0_1_0.md)

## Future ideas

- Full-lifecycle relationships and joins.
- External mapping bridges for complete, unmodifiable third-party Record types.
- Schema creation and migrations.
- Additional database backends.
- Async APIs.
- Broader native-platform and compiler validation.
- Publication to C++ package registries.
- Additional PostgreSQL-specific value types.

Future ideas do not have release numbers, priority, or internal milestones until work on them becomes current.
