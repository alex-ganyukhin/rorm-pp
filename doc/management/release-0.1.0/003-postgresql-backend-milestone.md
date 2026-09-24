# Milestone 003: PostgreSQL backend

## Objective

Connect to PostgreSQL and execute the internal structured operations needed by the `0.1.0` ORM.

## Expected outcome

PostgreSQL can execute backend operations with typed values, results, errors, and transactions before the public ORM layer exists.

## Completion criteria

- [ ] The internal operation model can express the database work required by the planned typed ORM.
- [ ] PostgreSQL executes those operations against an existing schema with values bound separately from generated SQL.
- [ ] Supported values and nulls round-trip between C++ and PostgreSQL; UUID columns map to `std::string`.
- [ ] Results map to scalar values, projections, or Records through the milestone 002 mapping machinery.
- [ ] An explicit transaction commits on request and rolls back on request or an uncommitted failure path.
- [ ] Subsystem tests demonstrate operation execution, result mapping, failures, commit, and rollback while satisfying the [project quality objectives](../goal.md#quality-objectives).

## Out of scope

- A public low-level operation-construction API.
- Parameterized raw SQL as a user-facing escape hatch.
- Typed ORM operations.
- Schema creation or migration.
- Async execution, connection pooling, other database backends, or first-class JSON/JSONB mapping.
