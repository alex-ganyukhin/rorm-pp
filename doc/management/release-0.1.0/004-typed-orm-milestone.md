# Milestone 004: Typed ORM

## Objective

Let applications perform common single-table database work through typed C++ operations instead of SQL strings.

## Expected outcome

A user can persist and query one Record type end to end while invalid C++ property/value combinations fail during compilation where possible.

## Completion criteria

- [ ] Individual insert, update, and delete operations work for a Record and return database-generated values where requested.
- [ ] Batch insert, update, and delete operations provide the same mapping behavior for collections of Records.
- [ ] A typed query can filter by Record properties, combine supported predicates, and bind values as parameters.
- [ ] A typed query can sort, apply limit/offset pagination, and return complete Records or selected typed properties.
- [ ] Typed count and exists operations return scalar results.
- [ ] Unsupported properties, values, and expression combinations fail during compilation where possible; database-schema mismatches return runtime errors.
- [ ] A PostgreSQL subsystem test performs the full individual and batch CRUD lifecycle plus every supported read category while satisfying the [project quality objectives](../goal.md#quality-objectives).

## Out of scope

- Joins, mapped relationships, aggregates beyond count, grouping, subqueries, and schema management.
- A user-facing raw-SQL escape hatch.
