# JSON GREEN execution progress

Original base: `b4011750c81b8342ff5500f454521fd3e5f3eab9`, branch `4-foundation-milestone`.
Execution branch: `json-green`, isolated under `build-worktrees/json-green` and visible at
`/workspaces/rorm-pp/build-worktrees/json-green` in GCC container `f4b5d5ee2010`.

## G0–G3 common base

- GCC 16.1.0 standalone reflection smoke passed.
- Original warning-clean configure/build passed. Original baseline: 79 checks, 10 passed and 69 failed.
- Approved policy struct replaced the enum across declarations, settled tests, negative fixtures, and examples; policy remains required.
- Structural names, function pointers, and stateful converters retained; backend-independent mapping validates field/accessor contracts.
- Original positive and eight negative compile gates observed RED baseline then GREEN: 9/9.
- Added four negative checks (duplicate getter/setter, getter arity, wrong converter error type) and retained-payload positive assertions.
- Real DOM backend, dependency propagation, and system includes in direct compile checks established.
- G2's four field/optional vertical slices and all named G3 rename/accessor/converter/directional cases passed.
- Added optional-initializer clearing, read-only alternate-type skipping, input ownership, and serialization-only construction cases.
- Formatted with standalone container clang-format. Explicit G1–G3 focused gate: 34/34 passed.
- Complete intermediate suite: 87 checks, 70 passed and 17 failed. Failures are pending parser duplicates, value families, codecs,
  and their combined/nested integration. This is **not** complete GREEN.

## Execution deviations and boundaries

The mutually dependent policy/mapping/backend foundation was batched into a common-base deliverable rather than separate G0.1/G1 commits.
The original suite provides the recorded RED evidence; not every new focused assertion was individually run before implementation.
The standalone policy-only migration baseline was not captured independently. These historical checks cannot be recreated after the fact.
One intermediate build/test attempt overlapped another while resolving compiler diagnostics; its shared log was discarded in favor of a
fresh serial authoritative run. Separate worker build trees are mandatory from the parallel wave onward.

Reflections make descriptors consteval-only. Static std::array materialization avoids structural NTTP restrictions; JSON copies names and
flags into constexpr scalar locals before runtime use. The frozen descriptor/API shape is unchanged.

Temporary failing branches remain in json_values.ipp/json_bytes.ipp/json_chrono.ipp pending G4; no successful placeholder data is returned.
Full Doxygen transfer, generic timestamp behavior, final coverage, final ordinary build, and final acceptance remain pending.

## Parallel ownership and interfaces

Each worker starts at the common-base commit and uses a separate worktree/build directory. Do not edit shared files or existing test_json.cc.
Request shared changes through the coordinator. All private declarations in json_impl.ipp are frozen to the plan's signatures.

| Worker | Exclusive files | Build directory |
| --- | --- | --- |
| G4A parser | detail/json_input.ipp, tests/test_json_input.cc | build-json-parser |
| G4B values | detail/json_values.ipp, tests/test_json_values.cc | build-json-values |
| G4C codecs | detail/json_bytes.ipp, detail/json_chrono.ipp, tests/test_json_codecs.cc | build-json-codecs |

Paths under detail are relative to rorm/include/rorm. The integrator owns json.h, json_impl.ipp, mapping/traits, CMake and existing tests.
The value worker must wire byte/chrono calls into its dispatcher; codec worker can test the private hooks standalone before integration.
The parser worker receives and returns the DOM through parse_json. encode_record/decode_record are already in coordinator-owned json_impl.ipp.
Value traits expose optional_traits<T>::element_type, sequence_traits<T>::element_type/fixed/size (size for arrays only), is_optional,
is_sequence, is_byte, byte_sequence<T>(), chrono_traits<T>::value/duration, expected_traits<T> and dependent_false<T>.
Annotation helpers logical_name(info), has_annotation<A>(info), annotation<A>(info) are backend independent and usable for enum reflection.
Do not pass property_descriptor itself as NTTP or use descriptor objects at runtime: copy scalar metadata into constexpr locals.

## Constitution check

Reflection-native mapping and nlohmann-independent reusable code preserved. Header-only target and existing error category preserved.
Policy migration and timestamp expansion are intentional approved contract changes. No performance claim or dependency upgrade introduced.
Runtime coverage and public documentation gates remain pending final acceptance; compile failures are evidence separate from runtime counters.

## Integration preparation

- Parser commit `9520996` reviewed PASS and integrated as `57edda5`.
- Value commit `df98663` integrated as `a35d511`; independent review pending.
- Foundation review found missing duplicate-conversion and `ignore` plus key/generated validation.
- Three focused compile-fail fixtures observed RED because invalid mappings compiled, then GREEN after shared mapping fix.
- Full mapping/positive focused gate: 20/20 passed (includes four existing mapping runtime cases).
- Separate `[[= ...]]` annotation lists are required by GCC; initial invalid fixture syntax was corrected before authoritative RED run.
- No whole-feature refactor has begun; codecs and full GREEN remain pending.
