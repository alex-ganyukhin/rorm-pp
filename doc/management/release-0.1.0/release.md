# Release 0.1.0

`0.1.0` is an early-adopter release that proves reflection-based Record mapping and typed PostgreSQL ORM operations. The [roadmap](./roadmap.md) defines the implementation order.

## Supported environment

- GCC 16.1 or newer with C++26 reflection enabled.
- Linux is the validated runtime and build environment.
- macOS development uses the Linux development container.
- Product code remains portable by design; native macOS and Windows validation depends on suitable reflection toolchains.

## Included features

### Record mapping

- Map public fields and explicitly marked getter/setter properties through standard reflection without macros or hand-written registration code.
- Support getter-only and setter-only properties where an operation needs only the available direction.
- Override mapped names, identify primary keys and database-generated values, exclude properties, and define custom value conversions.
- Reject invalid mappings and incompatible typed operations at compile time when the required information is available.
- Support booleans, fixed-width integers, floating-point values, strings, bytes, enums, optional values, common date/time values, and custom converted values.

### JSON interchange

- Serialize Records to JSON and deserialize them from JSON.
- Handle nested reflected values, optional values, and common sequences recursively.
- Apply mapped names, exclusions, accessor directions, and custom conversions consistently.
- Report malformed input, missing required values, and incompatible JSON values clearly.

### PostgreSQL and typed ORM

- Connect synchronously to an existing PostgreSQL schema.
- Execute typed ORM operations through an internal structured backend-operation layer.
- Control explicit transactions with commit and rollback behavior.
- Map supported C++ values to PostgreSQL; PostgreSQL UUID values use `std::string`.
- Insert, update, and delete individual Records and batches of Records, returning database-generated values.
- Build typed single-table filters and sorting expressions with parameter binding.
- Apply limit/offset pagination and return complete Records or selected typed properties.
- Execute typed count and exists operations.

### Distribution

- Install rorm as a CMake package consumable by another project without repository-relative paths.

## Unscheduled future features

- Parameterized raw SQL as a user-facing escape hatch.
- Full-lifecycle relationships and joins.
- External mapping bridges for complete third-party types that cannot be modified or annotated.
- Schema creation and migrations.
- Additional database backends.
- Async database APIs.
- First-class PostgreSQL JSON and JSONB column mapping.
- Additional PostgreSQL-specific value types.

Future features have no priority, release number, or detailed scope until work on them becomes current. Native platform expansion and publication to C++ package registries are also unscheduled.
