# Milestone 002: Record and JSON

## Objective

Implement the reflection-based Record mapping required to serialize and deserialize JSON through the public API planned in milestone 001.

## Expected outcome

A supported Record round-trips through JSON without a parallel schema, macros, or hand-written registration code.

## Detailed plan

- [ ] Derive named, typed, readable, and writable Record properties from public fields and explicitly marked accessors.
- [ ] Implement mapped names, exclusions, directional properties, and custom value conversions.
- [ ] Support booleans, fixed-width integers, floating-point values, strings, bytes, enums, optional values, common date/time values, nested reflected values, and common sequences.
- [ ] Serialize readable properties into valid JSON.
- [ ] Deserialize JSON into writable properties without silently accepting missing required or incompatible values.
- [ ] Produce actionable compile-time diagnostics for invalid mappings and runtime errors for malformed or incompatible JSON.
- [ ] Keep `api.md` Record and JSON examples synchronized with the delivered interface.
- [ ] Add unit and subsystem tests required by the [project quality objectives](../goal.md#quality-objectives).

## Completion criteria

- [ ] A representative Record combining fields, accessors, overrides, directional properties, nested values, optional values, and sequences round-trips without data loss.
- [ ] Getter-only properties serialize but are not assigned; setter-only properties are assigned but are not read during serialization.
- [ ] Invalid mappings fail during compilation with actionable diagnostics.
- [ ] Malformed JSON, missing required values, incompatible types, and conversion failures return clear errors without partial silent results.
- [ ] The public API and documentation match the milestone 001 contract or record an intentional correction to it.

## Out of scope

- A general-purpose serializer for arbitrary unrelated C++ types.
- First-class PostgreSQL JSON or JSONB column persistence.
- Database access.
