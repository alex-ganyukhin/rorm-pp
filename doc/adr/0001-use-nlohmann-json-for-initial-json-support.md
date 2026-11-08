---
Status: Active
Date: 2026-11-08
---
# Use nlohmann/json temporarily for initial JSON support

Initially, serialization follows
`C++ Record → nlohmann/json tree → JSON string`, and deserialization follows
`JSON string → nlohmann/json tree → C++ Record`.
This provides a straightforward starting point while rorm establishes its
public API, reflection-based mapping, and behavioral tests.

> [!WARNING]
> The initial serialization implementation is temporary and will be replaced
> with a substantially faster implementation. The initial implementation
> MUST be structured to support that replacement.

## Required implementation boundary

- rorm owns property discovery, annotations, accessor handling, conversions,
  and mapping validation independently of nlohmann/json.
- Shared mapping code must not depend on nlohmann/json types or its
  custom-type serialization machinery.
- Confine nlohmann/json tree construction, parsing, and string generation
  to the initial JSON implementation.
- A future serializer must be able to consume rorm's mapping directly and
  emit JSON without constructing a nlohmann/json tree.
- Preserve the public API and its behavioral contract when replacing
  serialization. Tests must not depend on the intermediate representation,
  incidental whitespace, or object-member order.

The future parsing backend is a separate decision.

## Alternatives considered

- Glaze overlaps with the reflection-based serialization functionality
  rorm intends to own.
- RapidJSON and direct string generation are deferred while the initial
  API and behavior are established.

## Consequences

The initial implementation accepts intermediate-tree overhead temporarily.
Its simplicity must not couple the reusable mapping machinery to that tree.
Performance measurements will validate the replacement's improvement.
