# Milestone 003: PostgreSQL execution

- **Status**: Planned

## Objective

Provide the synchronous PostgreSQL execution foundation used by typed ORM operations.

## Deliverables

- [ ] Connect to and execute commands against an existing PostgreSQL schema.
- [ ] Execute parameterized raw SQL with typed parameters.
- [ ] Map raw-query results into Records and scalar values.
- [ ] Provide explicit transactions with commit and rollback behavior.
- [ ] Map the practical core value set to PostgreSQL types.
- [ ] Support PostgreSQL UUID columns through `std::string`.

## Boundaries

- This milestone does not create or migrate schemas.
- Execution is synchronous and PostgreSQL-specific.
- PostgreSQL JSON and JSONB columns have no first-class mapping.

## Completion criteria

- Subsystem tests exercise successful execution, parameter binding, result conversion, database errors, commit, and rollback against PostgreSQL.
- Public APIs satisfy the [project quality objectives](../goal.md#quality-objectives).
