# Milestone 004: Typed ORM

- **Status**: Planned

## Objective

Deliver the typed, single-table persistence and query capabilities that make `0.1.0` a usable ORM.

## Deliverables

- [ ] Insert, update, and delete individual Records.
- [ ] Insert, update, and delete batches of Records.
- [ ] Return database-generated values to the caller.
- [ ] Filter and sort typed single-table queries.
- [ ] Apply limit/offset pagination.
- [ ] Select typed projections and evaluate count and exists queries.

## Boundaries

- Typed joins and mapped relationships are excluded.
- Parameterized raw SQL is the escape hatch for queries outside this milestone.
- Database-schema mismatches are reported at runtime; C++ mapping and operation mismatches are rejected during compilation where possible.

## Completion criteria

- Subsystem tests cover the complete individual and batch CRUD lifecycle against an existing PostgreSQL schema.
- Subsystem tests cover every supported typed-read category and relevant failures.
- Public APIs satisfy the [project quality objectives](../goal.md#quality-objectives).
