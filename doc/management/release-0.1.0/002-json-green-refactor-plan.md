# Milestone 002: JSON GREEN and REFACTOR Implementation Plan

> **For agentic workers:** Use `superpowers:subagent-driven-development` or `superpowers:executing-plans`.
> Track execution with the checkboxes below. Parallel workers must follow the ownership and dependency rules.

**Destination:** `doc/management/release-0.1.0/002-json-green-refactor-plan.md`

**File status:** G0–G3 common base implemented; G4 and acceptance gates pending.

**Goal:** Implement the settled Record/JSON contract, make the existing RED suite pass, then refactor without changing behavior.

**Architecture:** Derive and validate a backend-independent Record mapping through C++26 reflection. The initial JSON implementation uses
that mapping to construct and consume nlohmann/json trees. Keep parsing, JSON representation, and string generation inside the JSON backend.

**Tech stack:** GCC 16.1+, C++26 reflection, header-only CMake INTERFACE library, nlohmann/json 3.12.0, GoogleTest, FileCheck, gcovr.

**Sources of truth:** `doc/api.md`, milestone 002, ADRs 0001 and 0002, the project constitution, and the settled tests.

## 1. Agreed contract and execution rules

### Scope and compatibility

- [ ] Use the following public policy and signatures; keep the policy argument required:

  ```cpp
  struct deserialization_policy
  {
      bool ignore_unknown_members = false;
  };

  template<typename Record>
  std::expected<std::string, std::error_code> to_json( const Record& record );

  template<typename Record>
  std::expected<Record, std::error_code>
  from_json( std::string_view input, deserialization_policy policy );
  ```

- [ ] Preserve annotation factory signatures and supporting type identities checked by `CompileCheck.Positive`.
- [ ] Keep all annotations in `rorm/annotations.h`, including generic `rorm::base64`.
- [ ] Preserve JSON error values, category, and `make_error_code` behavior.
- [ ] Propagate converter-returned errors unchanged, including their category and value.
- [ ] Do not add diagnostic objects, paths, offsets, callbacks, or policy fields beyond `ignore_unknown_members`.
- [ ] Preserve `add_library(rorm INTERFACE)` and keep backend types out of supported public signatures and shared mapping code.
- [ ] Leave PostgreSQL, ORM behavior, dependency upgrades, benchmarks, and unrelated management documents outside this change.

### Decisions made during planning

- [ ] Preserve the settled RED behaviors and assertions while migrating policy arguments to the approved struct.
- [ ] Replace the old policy enum with `deserialization_policy`; do not add a legacy overload or implicit enum conversion.
- [ ] Default `ignore_unknown_members` to false. Pass `{}` to reject unknown members or
  `{ .ignore_unknown_members = true }` to ignore them; duplicates remain errors under both settings.
- [ ] Propagate the same policy recursively into nested Records.
- [ ] Add focused tests only for concrete contract gaps or new agreed behavior.
- [ ] Allow normal floating-point rounding when decoding floating-point values. Reject overflow, underflow to zero, and non-finite output.
- [ ] Keep stricter numeric conversion available through `rorm::as`; do not introduce a separate rounding-policy API.
- [ ] Generalize timestamp support to `std::chrono::sys_time<Duration>`, including custom periods.
- [ ] Accept arbitrary-length fractional seconds and round **directly to the destination duration**, using nearest/ties-to-even.
- [ ] Do not round through nanoseconds or a floating-point timestamp intermediate.

### Defaults for previously unspecified behavior

- [ ] Support `std::vector` and `std::array` as the initial sequence families. Do not infer support for every range or container.
- [ ] Treat `std::byte` and unsigned 8-bit elements as byte-like values.
- [ ] Apply `base64` to the logical byte sequence after custom conversion, including an optional logical byte sequence.
- [ ] Accept getter-only input members without decoding or assigning their values; parsing still validates their JSON syntax and encoding.
- [ ] Support integral timestamp representations up to 64 bits with positive `std::ratio` periods. Diagnose unsupported representations at
  compile time; floating-representation durations can use `rorm::as`.
- [ ] Use full calendar timestamps with seconds, optional fractional digits, and a required `Z` or `±HH:MM` offset.
- [ ] Use four-digit calendar years, reject invalid dates and leap-second spellings, and emit UTC.
- [ ] For timestamp periods with a finite decimal representation, emit the corresponding fractional width where practical, preserving the
  existing three-digit millisecond output.
- [ ] For repeating decimal periods, emit sufficient decimal precision to decode back to the same destination tick. Use a documented,
  deterministic rule rather than claiming an exact finite representation.

### TDD cycle for every implementation slice

- [ ] Select the existing test or focused group named below.
- [ ] Run it before implementation and record the relevant failure.
- [ ] Implement the smallest behavior that satisfies that test.
- [ ] Run the same test and all previously green tests affected by the change.
- [ ] Refactor locally only after the owned slice is green.
- [ ] Keep unrelated, still-RED cases visible in the handoff; do not describe a focused pass as a green suite.
- [ ] Commit each independently reviewable deliverable with its verification evidence.
- [ ] Remove all temporary backend scaffolds before declaring GREEN complete.

### Review focus

Each item has explicit coverage work in the owning task:

1. Duplicate keys and malformed encoding inside ignored or read-only subtrees.
2. Integer boundaries, float underflow/overflow, and accidental numeric coercion.
3. Directional properties, missing optional values with initializers, and unchanged converter errors.
4. Timestamp rounding at ties, negative instants, custom periods, rollover, and representation limits.
5. Base64 padding, incomplete groups, empty values, and fixed-size sequence lengths.

## 2. Starting point, architecture, and parallel ownership

### Verified starting point

The checkout was clean on branch `4-foundation-milestone`, at `b401175`.

| Check | Current result |
| --- | --- |
| GCC reflection smoke check | Passed with GCC 16.1.0 |
| Project configure and build | Passed |
| Complete CTest run | 79 checks: 10 passed, 69 failed |
| JSON runtime cases | 62 instances; 61 currently fail |
| Mapping compile-fail cases | 8; all currently fail because invalid mappings compile |
| Positive compile check | Passed |

The failures are the implementation starting point. Coverage has not been remeasured.

A throwaway compiler probe also established that structural annotation text can be retained with static storage, and overloaded accessors
can be invoked through reflected member-function pointers without weakening warnings.

### File responsibilities

Paths below `detail/` are relative to `rorm/include/rorm/detail/`.

| File or group | Responsibility |
| --- | --- |
| `rorm/annotations.h` | Retain structural annotation payloads; preserve public factories and types |
| `detail/value_traits.h` | Supported value-family traits and converter-result traits; no JSON dependency |
| `detail/record_mapping.h` | Discovery, grouping, validation, typed reads/writes, enum names; no JSON dependency |
| `detail/json_impl.ipp` | Backend declarations, include order, shared dispatch wiring |
| `detail/json_input.ipp` | Strict parsing, duplicate detection, numeric-token validation |
| `detail/json_values.ipp` | JSON scalar, optional, sequence, enum, and Record conversion |
| `detail/json_bytes.ipp` | Base64 text encoding and decoding |
| `detail/json_chrono.ipp` | Calendar and generic timestamp text conversion |
| `rorm/json.h` | Public declarations, errors, Doxygen, and thin entry-point orchestration |
| Root/test CMake and compile-check runner | Backend dependency propagation and test registration |
| Existing tests plus focused new test files | Acceptance tests and concrete gap coverage |
| `doc/api.md`, JSON quickstart | Transfer delivered contracts into Doxygen and update workflows |

The `.ipp` files are private implementation fragments included after their required declarations. They are not standalone public headers.
Their separation supports the three parallel work streams; it does not introduce a pluggable-backend framework.

### Shared internal interfaces

Freeze these before parallel work. All names are internal to `rorm::detail`.

```cpp
struct property_descriptor
{
    std::string_view name;
    std::meta::info reader;
    std::meta::info writer;
    bool base64;
    bool primary_key;
    bool generated;
};

template<typename Record>
struct record_mapping
{
    static constexpr auto properties = /* validated static descriptors */;
};

template<typename Record, std::meta::info Reader, std::meta::info Writer>
struct mapped_property
{
    using logical_type = /* normalized logical value type */;

    static constexpr bool readable = /* reader exists */;
    static constexpr bool writable = /* writer exists */;

    static std::expected<logical_type, std::error_code>
    read( const Record& record ); // Available only when readable.

    static std::expected<void, std::error_code>
    write( Record& record, logical_type value ); // Available only when writable.
};
```

Use null reflection values for absent directions. Iterate descriptors at compile time, passing their reader/writer reflections as template
arguments. Do not pass the complete descriptor as a template argument: its `std::string_view` member is nonstructural.

The initial owning `read` result keeps plain values and fallible conversions uniform. Do not introduce a reference-result abstraction
during GREEN.

Backend interfaces:

```cpp
using json_value = nlohmann::json;

std::expected<json_value, std::error_code>
parse_json( std::string_view input );

template<typename T>
std::expected<json_value, std::error_code>
encode_value( const T& value, bool base64 );

template<typename T>
std::expected<T, std::error_code>
decode_value( const json_value& value, deserialization_policy policy, bool base64 );

template<typename Bytes>
std::expected<std::string, std::error_code>
encode_base64( const Bytes& value );

template<typename Bytes>
std::expected<Bytes, std::error_code>
decode_base64( std::string_view text );

template<typename Chrono>
std::expected<std::string, std::error_code>
format_chrono( const Chrono& value );

template<typename Chrono>
std::expected<Chrono, std::error_code>
parse_chrono( std::string_view text );
```

Keep the backend alias and all functions mentioning it inside the JSON implementation. Forward-declare recursive value operations before
their definitions.

### Parallel execution

| Wave | Work | Ownership |
| --- | --- | --- |
| 0 | Baseline, mapping, backend bootstrap, core property behavior | Coordinator or one sequential implementer |
| 1A | Parsing and input validation | Parser worker |
| 1B | Numeric, enum, optional, and sequence values | Value worker |
| 1C | Bytes and generic chrono text | Text-codec worker |
| 2 | Integration and complete GREEN gate | Coordinator |
| 3 | Refactor, documentation, coverage, final acceptance | Coordinator; read-only reviews may run in parallel |

- [ ] Use isolated implementation worktrees, each visible inside the GCC development environment.
- [ ] Give every worker its own build directory.
- [ ] Create the common backend declarations, dispatch hooks, and supplemental test registrations before forking workers.
- [ ] Give each worker exclusive ownership of its implementation and supplemental test files.
- [ ] Reserve `json.h`, `json_impl.ipp`, shared mapping headers, CMake, and existing `test_json.cc` for the coordinator after the common base.
- [ ] Workers request shared-interface changes through the coordinator; they do not independently change neighboring contracts.
- [ ] Run final integration and coverage serially.
- [ ] Each worker reports its base revision, changed files, tests observed RED then GREEN, remaining failures, and integration requirements.

## 3. GREEN phase

### G0 — Record the baseline and prepare execution

**Owner:** Coordinator.  
**Deliverable:** A reproducible baseline and saved tracking document.

- [x] Save this plan at the destination above.
- [x] Recheck the working tree and preserve any unrelated changes.
- [x] Read the current ADRs, constitution, testing guide, and documentation guide before implementation.
- [x] Confirm that the design keeps reusable mapping independent of nlohmann/json.
- [x] Record the agreed timestamp expansion as an intentional change to the earlier millisecond-only planned contract.
- [x] Run these commands inside the GCC dev container:

  ```sh
  bash .devcontainer/post-create.sh
  cmake -S . -B build-json-green -G Ninja -DCMAKE_CXX_COMPILER=g++ -DCMAKE_BUILD_TYPE=Debug
  cmake --build build-json-green --parallel
  ctest --test-dir build-json-green --output-on-failure
  ```

- [x] Record failures by suite and confirm they represent missing behavior, not toolchain or dependency failures.
- [x] Keep this plan’s progress separate from existing milestone and roadmap checkboxes.

### G0.1 — Migrate the policy scaffold before GREEN implementation

**Owner:** Coordinator, before G1 and before parallel workers fork.  
**Files:** `rorm/include/rorm/json.h`, runtime/compile-check call sites, `doc/api.md`, and JSON quickstart.

- [x] Replace the enum declaration with the aggregate `deserialization_policy` defined above.
- [x] Change `from_json` to accept the struct by value, without a default argument.
- [x] Replace reject arguments with `{}` and ignore arguments with `{ .ignore_unknown_members = true }` throughout tests and examples.
- [x] Remove old enum imports and migrate every compile-fail fixture without changing its intended mapping diagnostic.
- [x] Update the result-type assertion to use the struct and preserve the assertion that omitting the policy is invalid.
- [x] Add compile-time checks that default construction rejects unknown members and designated initialization enables ignoring them.
- [x] Document the struct and its field locally with examples; explicitly preserve recursive handling and duplicate-member errors.
- [x] Update the planned API reference and quickstart call syntax in the same change.
- [ ] Configure/build and run the complete suite; confirm policy migration introduces no compilation regressions.
- [ ] Record the still-RED behavior baseline; this API migration does not implement JSON behavior.

### G1 — Implement structural annotations and backend-independent mapping

**Owner:** Mapping implementer, sequential.  
**Files:** `annotations.h`, `detail/value_traits.h`, `detail/record_mapping.h`; focused mapping tests and compile-fail additions.

**Primary GREEN gates:** `CompileCheck.Positive` and all eight existing `JsonMapping.*` checks.

#### Retain annotation payloads

- [x] Run `CompileCheck.Positive` before changing annotation storage.
- [x] Preserve `name_annotation`, `getter_annotation`, `setter_annotation`, and `table_annotation` as the existing public types.
- [x] Retain text using structural pointer/length payloads backed by promoted static character storage.
- [x] Use `std::define_static_string` in the consteval factories; retain the supplied length.
- [x] Keep `std::string_view` as the factory input, without storing it inside annotation values.
- [x] Preserve `conversion_annotation<Shared, Converters...>` type identities.
- [x] Store one or two converter values directly in structural specializations; do not store them in `std::tuple`.
- [x] Retain function-pointer values and structural functor state instead of default-constructing converter types.
- [x] Add focused positive checks for retained names, function pointers, and a structural callable with state.
- [x] Re-run the positive compile check.

#### Discover and group properties

- [x] Discover directly declared public nonstatic fields and explicitly marked public methods.
- [x] Exclude private state, static fields, unmarked methods, constructors, destructors, and implicit special members.
- [x] Use the normal access context and public filtering; do not reuse the test printer’s unchecked private access.
- [x] Inspect annotation types before extraction and ignore unrelated third-party annotations.
- [x] Apply `name` overrides and remove ignored fields from the logical mapping.
- [x] Pair one getter and one setter sharing a logical name.
- [x] Normalize getter results, setter arguments, and conversion logical types with `std::remove_cvref_t`.
- [x] Retain ORM metadata without letting key/generated annotations change JSON inclusion.
- [x] Materialize reflected member-function pointers and invoke those pointers. This handles overloaded accessors without textual lookup.

#### Validate before instantiating operations

- [x] Validate getter shape: public, nonstatic, const, no explicit arguments, nonvoid result, callable on `const Record&`.
- [x] Validate setter shape: public, nonstatic, exactly one explicit argument, callable on `Record&`.
- [x] Do not require setters to return `void`; the existing contract does not impose that restriction.
- [x] Validate converter invocability before instantiating result-type queries.
- [x] Accept destination values or `std::expected<Destination, std::error_code>` as converter results.
- [x] Validate both directions of field conversions, even when the immediate operation uses only one.
- [x] Validate paired accessor logical-type agreement.
- [x] Validate base64 applicability against the logical byte-sequence type.
- [x] Reject contradictory mapping annotations rather than selecting one by source order.
- [x] Instantiate structural validation from both JSON entry points.

Preserve the existing diagnostic substrings:

| Existing compile-fail check | Required substring |
| --- | --- |
| `JsonMapping.duplicate_name` | `duplicate` |
| `JsonMapping.field_accessor_collision` | `duplicate` |
| `JsonMapping.non_const_getter` | `getter` |
| `JsonMapping.setter_arity` | `setter` |
| `JsonMapping.accessor_type_mismatch` | `type` |
| `JsonMapping.ignore_with_name` | `ignore` |
| `JsonMapping.base64_on_integer` | `base64` |
| `JsonMapping.invalid_converter` | `convert` |

- [ ] Run each existing negative check before implementing its validation rule.
- [x] Make it fail compilation for the intended reason, then confirm FileCheck passes.
- [x] Add focused negative fixtures for duplicate getters/setters, invalid getter arity, and wrong converter error types.
- [x] Add positive coverage for valid paired converted accessors.
- [x] Confirm a serialization-only Record does not acquire a default-construction requirement.
- [x] Keep deserialization’s `Record {}` requirement local to deserialization.

#### Implement typed property operations

- [x] Read fields or getters, then apply stored-to-logical conversion when present.
- [x] Decode logical values before applying logical-to-stored conversion and assignment.
- [x] Move decoded values where appropriate.
- [x] Normalize plain and expected converter results once.
- [x] Forward converter errors unchanged.
- [x] Let allocation failures and user-code exceptions propagate according to the existing contract.
- [x] Document non-obvious mapping invariants locally.

**Gate:**

```sh
ctest --test-dir build-json-green --output-on-failure -R '^(CompileCheck\.Positive|JsonMapping\.)'
```

All selected checks must pass. Runtime JSON tests may still be RED.

### G2 — Wire the backend and make the first vertical slice GREEN

**Owner:** Coordinator.  
**Files:** Root/test CMake, compile-check runner, `json.h`, backend implementation fragments.

**Deliverable:** Public-field JSON exchange through the real backend, with stable interfaces for parallel workers.

#### Dependency and compile-check wiring

- [x] Move the existing nlohmann FetchContent declaration from test-only configuration to root configuration.
- [x] Keep version 3.12.0 and `SYSTEM` treatment.
- [x] Propagate its headers through:

  ```cmake
  target_link_libraries(rorm INTERFACE nlohmann_json::nlohmann_json)
  ```

- [x] Retain the tests’ explicit nlohmann dependency because they directly use the JSON oracle.
- [x] Supply evaluated backend include directories as system includes to both direct compiler-check paths.
- [x] Preserve FileCheck diagnostics, locale control, and project warning flags.
- [x] Verify that negative checks cannot pass merely because a backend include is missing.

#### Backend bootstrap

- [x] Keep public error declarations available before including implementation fragments.
- [x] Preserve error-code specialization ordering and inline/header-only linkage.
- [x] Establish the private interfaces listed above.
- [x] Prepare separate bytes and chrono hooks so the text worker owns their implementations.
- [x] Keep temporary unimplemented branches explicitly failing; do not return successful placeholder data.
- [x] Prepare focused supplemental test files and register them in CMake before parallel work starts.

#### Minimal field round-trip

Use these existing tests in order:

- [x] `GIVEN_public_fields_WHEN_serialized_THEN_all_fields_are_present`
- [x] `GIVEN_reordered_members_WHEN_deserialized_THEN_fields_match`
- [x] `WHEN_record_round_trips_THEN_all_fields_are_preserved`
- [x] `GIVEN_missing_or_null_optional_WHEN_deserialized_THEN_optional_is_disengaged`

Implementation steps:

- [x] Construct an object tree and serialize readable public fields using their logical names.
- [x] Implement the string, integer, and optional behavior needed by `Profile`.
- [x] Parse the complete input range into a JSON tree.
- [x] Require an object for Record input.
- [x] Assign fields by logical name, independent of input ordering.
- [x] Require nonoptional writable properties despite C++ initializers.
- [x] Assign an empty optional when its input member is missing or null.
- [x] Return the local Record only after all required assignments succeed.
- [x] Return an owning output string and retain no input views.

**Gate:** The four named tests and the complete compile-check group pass. Preserve all initially passing support/reflection tests.

### G3 — Make annotated and directional properties GREEN

**Owner:** Coordinator or mapping implementer.  
**Dependency:** G1 and G2.

Run the existing tests in these slices:

| Slice | Existing tests |
| --- | --- |
| Renames/exclusions | `GIVEN_Renamed_WHEN_serialized_THEN_json_matches` |
| | `GIVEN_mapping_annotations_WHEN_deserialized_THEN_renames_and_exclusions_apply` |
| | `WHEN_unmapped_cpp_names_are_read_THEN_unknown_member_is_returned` |
| | `WHEN_renamed_property_has_wrong_type_THEN_input_is_rejected` |
| Accessors | `WHEN_accessors_are_serialized_THEN_only_readable_properties_are_present` |
| | `WHEN_accessors_are_deserialized_THEN_writable_state_matches` |
| | `WHEN_readonly_property_is_absent_THEN_deserialization_succeeds` |
| | `WHEN_setteronly_property_is_absent_THEN_missing_property_is_returned` |
| Conversions | `GIVEN_Converted_WHEN_serialized_THEN_json_matches` |
| | `WHEN_converted_id_is_read_THEN_custom_conversion_applies` |
| | `WHEN_decode_conversion_fails_THEN_original_error_is_returned` |
| | `WHEN_encode_conversion_fails_THEN_original_error_is_returned` |
| | `WHEN_converted_accessors_round_trip_THEN_id_is_preserved` |

- [ ] Run each slice while RED before wiring its operations.
- [x] Use one logical mapping for serialization, accepted input names, and required-property checks.
- [x] Serialize getter-only properties and omit setter-only properties.
- [x] Accept getter-only input names without assignment; do not require their presence.
- [x] Require setter-only properties unless their logical type is optional.
- [x] Keep excluded and original renamed C++ names outside the accepted logical-name set.
- [x] Route field and accessor conversions through the shared mapping operations.
- [x] Confirm encode/decode errors retain the original `std::errc` values tested by RED.
- [x] Add a focused test showing a missing optional overrides an engaged C++ initializer.
- [x] Add a focused test showing a present read-only property is skipped after syntax validation, even with another JSON value type.
- [x] Add a focused input-lifetime test that destroys or mutates the original input storage after successful decoding.
- [x] Re-run the mapping compile checks after accessor/converter integration.

**Gate:** All named tests and the G2 tests pass. Freeze shared interfaces and fork the parallel workers.

### G4A — Harden parsing and input validation

**Owner:** Parser worker.  
**Files:** `detail/json_input.ipp`, parser-specific supplemental tests.  
**Dependency:** G3 common base.

**Approach:** Use the public nlohmann SAX interface for validation, then construct the normal nlohmann DOM. The temporary two-pass cost keeps
duplicate and numeric-token validation simple without depending on nlohmann internals.

- [ ] Run the existing malformed-input and duplicate-member tests while RED.
- [ ] Track decoded keys separately for each open object.
- [ ] Detect duplicates before DOM construction can overwrite them.
- [ ] Detect duplicates globally, including ignored members, unknown nested subtrees, and objects inside arrays.
- [ ] Validate the entire input and reject trailing content.
- [ ] Preserve string lengths, including embedded NUL characters.
- [ ] Validate UTF-8 and escaped Unicode through the backend’s strict parser.
- [ ] Map malformed syntax/encoding to `malformed_input`.
- [ ] Map backend numeric overflow and detected numeric underflow to `out_of_range`.
- [ ] Reject integer tokens beyond the backend’s signed/unsigned 64-bit integer domain instead of silently promoting them to floating point.
- [ ] Check floating tokens before tree construction where the backend would otherwise lose underflow information.
- [ ] Catch only backend failures at backend boundaries; do not encompass user converters or setters in translation catches.
- [ ] Return the first detected failure without introducing public diagnostic detail.

Existing gates:

- [ ] `WHEN_malformed_json_is_read_THEN_malformed_input_is_returned`
- [ ] `GIVEN_ignore_policy_WHEN_duplicate_member_is_read_THEN_duplicate_member_is_returned`
- [ ] Parameterized `DuplicateMember`
- [ ] Preserve the basic successful parsing tests from G2.

Focused additions, written before their fixes:

- [ ] Duplicate keys inside an ignored nested object.
- [ ] Duplicate keys in an object inside an array.
- [ ] Equivalent decoded key spellings, such as `"name"` and `"\u006eame"`.
- [ ] Invalid raw UTF-8 and malformed syntax inside an ignored subtree.
- [ ] Integer tokens beyond signed/unsigned 64-bit bounds.
- [ ] Floating overflow and underflow-to-zero tokens.

**Handoff:** Parser-focused tests pass; report unrelated RED cases separately.

### G4B — Complete supported value families

**Owner:** Value worker.  
**Files:** `detail/json_values.ipp`, value-specific supplemental tests.  
**Dependency:** G3 common base.

#### Scalar categories and numbers

- [ ] Keep booleans distinct from numeric values.
- [ ] Require compatible JSON scalar categories; do not coerce strings or booleans into numbers.
- [ ] Preserve signed and unsigned integer values without a floating-point intermediate.
- [ ] Range-check before integral conversion.
- [ ] Reject fractional JSON numbers assigned to integral properties.
- [ ] Decode float/double with normal rounding, while rejecting overflow and nonzero values that become zero.
- [ ] Preserve representable subnormal values.
- [ ] Reject NaN and infinities before JSON output can turn them into null.
- [ ] Preserve string contents and reject invalid UTF-8 output with `incompatible_value`.

Existing gates:

- [ ] `GIVEN_Text_WHEN_serialized_THEN_json_matches`
- [ ] `WHEN_escaped_text_is_deserialized_THEN_unicode_and_controls_are_preserved`
- [ ] `GIVEN_Numbers_WHEN_serialized_THEN_json_matches`
- [ ] `WHEN_numeric_boundaries_are_deserialized_THEN_values_match`
- [ ] `WHEN_integer_is_out_of_range_THEN_out_of_range_is_returned`
- [ ] `WHEN_nonfinite_number_is_serialized_THEN_incompatible_value_is_returned`
- [ ] All ten `JsonNumericTypes` instances.
- [ ] Parameterized `NullRequired`, `StringNumber`, `BoolNumber`, `FractionalInteger`, and `IntegerOverflow`.
- [ ] Parameterized `ArrayRoot` and `NullRoot`.

Focused additions:

- [ ] Finite double-to-float rounding succeeds.
- [ ] Float-target overflow and underflow-to-zero fail.
- [ ] A representable subnormal succeeds.
- [ ] Invalid UTF-8 in a stored string fails serialization.
- [ ] Scalar mismatch directions not represented by the settled cases fail without coercion.

#### Enums

- [ ] Reflect enumerators through backend-independent enum metadata.
- [ ] Apply `rorm::name` overrides in both directions.
- [ ] Reject unknown names and unmapped enum values with `incompatible_value`.
- [ ] Reject duplicate effective enum names at compile time.

Existing gates:

- [ ] `GIVEN_EnumRecord_WHEN_serialized_THEN_json_matches`
- [ ] `WHEN_enum_name_is_read_THEN_enumerator_matches`
- [ ] `WHEN_unknown_enum_name_is_read_THEN_incompatible_value_is_returned`

Focused additions:

- [ ] Decode the renamed `"waiting"` enumerator.
- [ ] Encode an ordinary enumerator.
- [ ] Reject an invalid underlying enum value on output.

#### Optional, sequences, and nested Records

- [ ] Dispatch optional contents recursively and serialize disengaged optionals as null.
- [ ] Decode vectors preserving element order.
- [ ] Decode arrays only when input length matches exactly.
- [ ] Handle byte elements as range-checked integers for default array representation.
- [ ] Recurse through the same Record mapping and value functions.
- [ ] Propagate unknown-member policy into every nested Record.
- [ ] Return nested errors unchanged and stop producing a result on failure.
- [ ] Reject unsupported value families at compile time rather than leaving a generic runtime failure fallback.

Existing gates:

- [ ] `GIVEN_Nested_WHEN_serialized_THEN_json_matches`
- [ ] `WHEN_nested_records_are_deserialized_THEN_sequence_matches`
- [ ] `WHEN_nested_required_field_is_missing_THEN_missing_property_is_returned`
- [ ] `WHEN_nested_unknown_member_is_read_THEN_selected_policy_applies`
- [ ] `WHEN_array_length_is_wrong_THEN_incompatible_value_is_returned`

Focused additions:

- [ ] Empty vector success.
- [ ] Fixed-array inputs that are too long.
- [ ] Wrong container kind and incompatible sequence element.
- [ ] Byte values below zero or above 255.
- [ ] An error in a later sequence element returns no successful partial Record.

**Handoff:** Owned scalar/enum/container tests pass; byte/base64 integration waits for G4C.

### G4C — Implement bytes and generic chrono text

**Owner:** Text-codec worker.  
**Files:** `detail/json_bytes.ipp`, `detail/json_chrono.ipp`, bytes/chrono supplemental tests.  
**Dependency:** G3 common base.

#### Base64

- [ ] Run the existing base64 tests before implementation.
- [ ] Implement standard base64 output with correct final-group padding.
- [ ] Decode strictly: reject invalid characters, misplaced padding, impossible lengths, and nonzero unused trailing bits.
- [ ] Support empty values and both byte element representations.
- [ ] Enforce fixed-size output container length when decoding into an array.
- [ ] Return `incompatible_value` for malformed base64.
- [ ] Keep annotation applicability validation in the shared mapping, outside the codec.

Existing integration gates:

- [ ] `GIVEN_Bytes_WHEN_serialized_THEN_json_matches`
- [ ] `WHEN_byte_sequences_are_deserialized_THEN_values_match`
- [ ] `WHEN_invalid_base64_is_read_THEN_incompatible_value_is_returned`
- [ ] `WHEN_array_length_is_wrong_THEN_incompatible_value_is_returned`

Focused additions:

- [ ] Successful empty base64.
- [ ] One-byte and two-byte payloads requiring padding.
- [ ] Invalid padding placement, invalid trailing bits, and incomplete groups.
- [ ] Base64 through an accessor and through a logical-value conversion.
- [ ] Optional byte values preserve null/missing semantics.

#### Calendar dates

- [ ] Parse and format `std::chrono::year_month_day` as `YYYY-MM-DD`.
- [ ] Validate the calendar date before constructing or formatting the result.
- [ ] Reject invalid calendar values with `incompatible_value`.

Existing gates:

- [ ] `GIVEN_Dates_WHEN_serialized_THEN_json_matches`
- [ ] `WHEN_invalid_date_is_read_THEN_incompatible_value_is_returned`

Focused additions:

- [ ] Valid leap day.
- [ ] Invalid leap day.
- [ ] Invalid stored date on output.

#### Generic timestamps

This is newly agreed behavior beyond the settled millisecond RED tests. Add its focused tests before implementing it.

- [ ] Recognize `sys_time<Duration>` through a trait; do not enumerate seconds/milliseconds/microseconds/nanoseconds in the codec.
- [ ] Validate the complete calendar/time/offset spelling.
- [ ] Split input into whole UTC seconds and a borrowed decimal fractional view.
- [ ] Accept any positive number of fractional digits; reject a bare decimal point.
- [ ] Normalize offsets before rounding to the destination tick grid.
- [ ] Compute the two neighboring destination ticks with checked quotient/remainder arithmetic.
- [ ] Compare the original decimal fraction against the exact rational midpoint.
- [ ] Use decimal digit comparison/long division when needed; do not accumulate an unbounded power of ten or convert the fraction to float.
- [ ] Round once to the nearest tick; select the even epoch-relative tick at an exact tie.
- [ ] Handle negative instants with floor-based decomposition.
- [ ] Carry rounded results across second, day, month, and year boundaries.
- [ ] Check destination representation limits before conversion or increment.
- [ ] Return `out_of_range` for a valid timestamp outside the destination or supported calendar range.
- [ ] Emit UTC, retaining exact standard decimal widths such as 3, 6, and 9 digits.
- [ ] For other finite decimal periods, derive width from the period.
- [ ] For repeating periods, generate a deterministic decimal approximation sufficient for same-tick round-trip.
- [ ] Document the repeating-period output approximation and test the round-trip guarantee.
- [ ] Do not add a multiprecision dependency or a public rounding option.

Existing gate:

- [ ] `WHEN_timestamp_has_offset_THEN_deserialization_normalizes_to_utc`

New timestamp test matrix:

- [ ] Seconds, milliseconds, microseconds, and nanoseconds.
- [ ] Custom decimal periods, including finer-than-nanosecond precision near the epoch.
- [ ] A nondecimal period such as one-third of a second.
- [ ] Input without a fraction and with shorter/longer fractions.
- [ ] Fractions with long zero tails and a final nonzero digit.
- [ ] Below-half, exact-half-even, exact-half-odd, and above-half values.
- [ ] Regression: `.0005000001` seconds rounds directly to `1 ms`.
- [ ] Equivalent tie cases before the epoch.
- [ ] Positive/negative offsets and date rollover.
- [ ] Rounding into the next second and year.
- [ ] Representation minimum/maximum and rounding overflow.
- [ ] Invalid time fields, missing offset, malformed fraction, and trailing characters.
- [ ] Re-encoding preserves the existing millisecond string `"2026-09-20T10:30:15.125Z"`.

**Handoff:** Owned standalone codec cases pass; combined `Bytes` tests pass after the value worker’s changes are integrated.

### G5 — Integrate and close the complete GREEN gate

**Owner:** Coordinator.  
**Dependency:** G4A, G4B, G4C.

- [ ] Review worker diffs and integrate their owned files.
- [ ] Resolve interface differences in the coordinator-owned integration fragment.
- [ ] Remove temporary scaffolds and unsupported runtime fallback branches.
- [ ] Confirm no nlohmann includes or types occur in shared mapping/traits.
- [ ] Confirm both unknown-member policies use the full logical-name set.
- [ ] Confirm duplicate detection is independent of unknown-member policy.
- [ ] Confirm read-only input names are accepted and setter-only properties are required when nonoptional.
- [ ] Confirm ORM key/generated annotations do not remove JSON properties.
- [ ] Confirm converter calls are outside backend exception-translation boundaries.
- [ ] Confirm no partial successful result escapes on a late failure.

Run these final existing integration gates:

- [ ] `GIVEN_ignore_policy_WHEN_unknown_member_is_read_THEN_known_fields_match`
- [ ] Parameterized `UnknownMember`
- [ ] `WHEN_nested_unknown_member_is_read_THEN_selected_policy_applies`
- [ ] `GIVEN_combined_mapping_WHEN_round_tripped_THEN_state_is_preserved`

Then run:

```sh
cmake --build build-json-green --parallel
ctest --test-dir build-json-green --output-on-failure
```

- [ ] All original 79 checks pass.
- [ ] All focused additions pass.
- [ ] The build remains clean under `-Wall -Wextra -Werror`.
- [ ] Record the actual final test count and results.
- [ ] Do not begin whole-feature REFACTOR while any test remains failing.

## 4. REFACTOR phase

### R1 — Simplify mapping and conversion code

**Owner:** Coordinator.  
**Entry condition:** Complete suite GREEN.

- [ ] Capture a complete passing test run before structural changes.
- [ ] Consolidate duplicate annotation extraction or property grouping only where duplication exists.
- [ ] Keep one source for names, directionality, logical types, and representation metadata.
- [ ] Consolidate plain/expected converter-result normalization.
- [ ] Remove unnecessary copies where straightforward, without introducing reference wrappers or ownership abstractions.
- [ ] Preserve actionable compile-time diagnostics.
- [ ] Preserve validation of mappings from either public operation.
- [ ] Re-run mapping compile checks and affected runtime cases after each refactor.
- [ ] Run the complete suite before committing this refactor.

### R2 — Simplify backend implementation

**Owner:** Coordinator.

- [ ] Consolidate repeated numeric range checks where their semantics are identical.
- [ ] Keep recursive optional/sequence/Record handling in the shared value functions.
- [ ] Keep base64 and chrono text details outside the central value dispatcher.
- [ ] Remove duplicated unknown-member and required-property handling.
- [ ] Remove dead scaffolds, stale comments, unused includes, and speculative extension points.
- [ ] Keep the initial parser validation pass unless a simpler implementation preserves every tested guarantee.
- [ ] Do not replace the backend, introduce streaming output, or make performance claims in this phase.
- [ ] Run the complete suite after each independently meaningful refactor.

### R3 — Transfer delivered contracts into Doxygen

**Owner:** Coordinator.  
**Files:** `annotations.h`, `json.h`, internal contracts, `doc/api.md`, `doc/quickstart/json.md`.

- [ ] Replace scaffold descriptions with implemented behavior.
- [ ] Document supported value families, directionality, required/optional rules, ownership, and error propagation.
- [ ] Document normal floating-point rounding and the `rorm::as` escape hatch.
- [ ] Document generic timestamp representations, arbitrary fractional input, direct ties-to-even rounding, and output rules.
- [ ] Distinguish returned errors from allocation and user-code exceptions.
- [ ] Give changed public APIs and independently usable annotations local examples.
- [ ] Keep ORM-only semantics clearly provisional.
- [ ] Retire duplicated delivered Record/JSON reference material from `doc/api.md`.
- [ ] Preserve decision rationale and unimplemented PostgreSQL/ORM content.
- [ ] Update JSON quickstart availability wording and links.
- [ ] Leave the quickstart as a practical workflow rather than another API reference.
- [ ] Compile changed Doxygen examples with the supported compiler.
- [ ] Compile and run the JSON quickstart through a consumer using the `rorm` target.
- [ ] Wrap documentation at 140 columns and code/examples at 120.
- [ ] Review Doxygen alignment manually after formatting.
- [ ] Check the constitution’s public-API documentation gate explicitly.

### R4 — Coverage, formatting, and final acceptance

**Owner:** Coordinator.

- [ ] Format changed C++ headers, implementation fragments, and test files with standalone dev-container `clang-format`.
- [ ] Run formatter dry checks and `git diff --check`.
- [ ] Confirm new source/CMake/script files carry the repository copyright/license header.
- [ ] Run the standalone reflection smoke check.
- [ ] Configure, build, and run the complete ordinary test suite.
- [ ] Configure a separate coverage build:

  ```sh
  cmake -S . -B build-json-coverage -G Ninja \
    -DCMAKE_CXX_COMPILER=g++ -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON
  cmake --build build-json-coverage --target coverage
  ```

- [ ] Do not run other tests concurrently in that coverage tree.
- [ ] Inspect the generated line/branch report; target success alone does not enforce percentage gates.
- [ ] Verify line and branch coverage are above 95%, following the stricter repository checklist.
- [ ] Inspect production headers separately so test-source coverage cannot mask untested implementation.
- [ ] Account for compile-time validation through positive/negative compile evidence; it has no runtime counters.
- [ ] For a concrete uncovered runtime branch, add a focused test that demonstrates the relevant behavior.
- [ ] If a coverage addition already passes, verify its sensitivity with a temporary local fault and restore the implementation.
- [ ] Do not add meaningless tests, suppress branches, or exclude implementation merely to raise coverage.
- [ ] If a gate remains unmet or unmeasurable, report the exact gate, evidence, and reason; do not mark it passed.
- [ ] Review the final diff for unrelated edits and generated/dependency files.
- [ ] Complete the post-design/post-implementation Constitution Check.
- [ ] Record compiler, build, test, coverage, documentation, and example-validation results separately.

## 5. Completion and handoff

Mark completion only when all applicable items are evidenced:

- [ ] Every original RED test passes without weakened assertions.
- [ ] New timestamp behavior and concrete contract gaps have focused coverage.
- [ ] Invalid mappings fail with actionable diagnostics.
- [ ] Backend types remain confined to JSON implementation code.
- [ ] Public signatures match the approved policy migration; annotation identities, explicit policy, and error propagation are preserved.
- [ ] Generic timestamps round once, directly, with nearest/ties-to-even semantics.
- [ ] No temporary scaffolds remain.
- [ ] Public Doxygen and workflow documentation describe delivered behavior.
- [ ] Required formatting, warning, test, coverage, and documentation gates pass.
- [ ] The progress file includes task results, verification evidence, and any unresolved limitations.

G0–G3 evidence and remaining limitations are recorded in `002-json-green-progress.md`. Complete GREEN, refactor, coverage, and final
documentation acceptance remain pending; temporary backend branches are explicitly failing scaffolds.
