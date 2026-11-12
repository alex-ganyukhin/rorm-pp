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

At the foundation handoff, value/byte/chrono fragments still had explicit failing branches. G4 replaced these; no temporary backend
scaffolds remain in the integrated implementation. Later sections record delivered behavior and final acceptance separately.

## Parallel ownership and interfaces

Each worker starts at the common-base commit and uses a separate worktree/build directory. Do not edit shared files or existing
test_json.cc.
Request shared changes through the coordinator. All private declarations in json_impl.ipp are frozen to the plan's signatures.

| Worker | Exclusive files | Build directory |
| --- | --- | --- |
| G4A parser | detail/json_input.ipp, tests/test_json_input.cc | build-json-parser |
| G4B values | detail/json_values.ipp, tests/test_json_values.cc | build-json-values |
| G4C codecs | detail/json_bytes.ipp, detail/json_chrono.ipp, tests/test_json_codecs.cc | build-json-codecs |

Paths under detail are relative to rorm/include/rorm. The integrator owns json.h, json_impl.ipp, mapping/traits, CMake and existing tests.
The value worker must wire byte/chrono calls into its dispatcher; codec worker can test the private hooks standalone before integration.
The parser worker receives and returns the DOM through parse_json. encode_record/decode_record are already in coordinator-owned
json_impl.ipp.
Value traits expose optional_traits<T>::element_type, sequence_traits<T>::element_type/fixed/size (size for arrays only), is_optional,
is_sequence, is_byte, byte_sequence<T>(), chrono_traits<T>::value/duration, expected_traits<T> and dependent_false<T>.
Annotation helpers logical_name(info), has_annotation<A>(info), annotation<A>(info) are backend independent and usable for enum reflection.
Do not pass property_descriptor itself as NTTP or use descriptor objects at runtime: copy scalar metadata into constexpr locals.

## Constitution check

Reflection-native mapping and nlohmann-independent reusable code preserved. Header-only target and existing error category preserved.
Policy migration and timestamp expansion are intentional approved contract changes. No performance claim or dependency upgrade introduced.
Runtime coverage and public documentation gates remain pending final acceptance; compile failures are evidence separate from runtime
counters.

## Integration preparation

- Parser commit `9520996` reviewed PASS and integrated as `57edda5`.
- Value commit `df98663` integrated as `a35d511`; independent review pending.
- Foundation review found missing duplicate-conversion and `ignore` plus key/generated validation.
- Three focused compile-fail fixtures observed RED because invalid mappings compiled, then GREEN after shared mapping fix.
- Full mapping/positive focused gate: 20/20 passed (includes four existing mapping runtime cases).
- Separate `[[= ...]]` annotation lists are required by GCC; initial invalid fixture syntax was corrected before authoritative RED run.
- No whole-feature refactor has begun; codecs and full GREEN remain pending.

## G5 combined GREEN

- Codecs `9db8583` integrated as `646bd1e`; all three worker streams now integrated.
- Independent value review found finite long-double output could narrow to null/zero. Focused regression observed RED, then checked
  double narrowing returns `out_of_range` for overflow/underflow-to-zero and preserves `incompatible_value` for nonfinite source values.
- Independent codec review found out-of-calendar UTC input could round back into range. Focused regression observed RED; both normalized
  source calendar and rounded destination calendar are now checked.
- Complete fresh serial warning-clean build/test after both fixes: **122/122 PASS**. Original RED assertions remain intact.
- An earlier intermediate regression run overlapped test rebuilding; it is discarded as acceptance evidence. The fresh serial log
  `review-fixes-tests.log` is authoritative.

## R1–R2 refactor

- Moved backend-independent enum metadata/duplicate-name validation from JSON dispatcher into shared Record mapping.
- Existing owning member reads and converter-result normalization already have one implementation; no reference wrapper or extra abstraction
  added. Required-property and unknown-member handling each have one owner; numeric signed/unsigned/float checks have distinct semantics.
- Inspected backend fragments: no temporary scaffolds or generic runtime fallback remain. Recursive value handling stays centralized,
  text codecs stay separate, and the strict SAX prevalidation pass remains. No backend replacement or performance claim.
- Standalone clang-format applied; complete post-refactor suite **122/122 PASS**, warning-clean build.

## R3 documentation delivery

Documentation commit `8041423` transfers delivered contracts to Doxygen and retires duplicated Record/JSON reference text while preserving
provisional ORM/PostgreSQL content. All 15 local Doxygen examples compile through a consumer linked to `rorm` under GCC 16.1; the JSON
quickstart consumer compiles/runs and verifies the Ada/null/visits=2 round trip. Dedicated build: `build-json-docs`. All 15 independent
changed public surface items have local documentation/examples; supporting annotation types reference their factories as permitted.
Doxygen HTML tooling was not added; the public-documentation gate is source/example inspection. Internal comments document owning mapping
reads, null direction reflections, and the decimal fraction comparison preconditions.

## R4 final acceptance and final-review fix

- Broad final review found accessor conversions accepted a second unused converter. Getter/setter focused negative fixtures both observed
  RED (compiled successfully), then passed after requiring exactly one accessor converter. Field conversions still require two.
- Four coverage-driven behavior additions cover full base64 alphabet, late sequence-output failure, optional input failure, and invalid
  calendar output. All four passed correct code, each failed with its corresponding temporary local fault; all faults restored.
- Standalone reflection smoke: PASS, GCC 16.1.0.
- Standalone clang-format dry checks: PASS. Host `git diff --check`: PASS. New source/license headers: verified.
- Ordinary configure/build: PASS, unchanged `-Wall -Wextra -Werror`. Complete ordinary suite: **128/128 PASS** (110 runtime + 18 compile
  checks).
- Separate coverage configure/target: PASS; its complete suite also **128/128 PASS**. No concurrent test process used that coverage tree.
- Overall coverage: **98.6% lines (1135/1151)**, **37.4% branches (1746/4672)**, 80.7% functions (943/1169).
- Production-only coverage: **96.6% lines (459/475)**, **67.4% branches (578/857)**, 96.1% functions (439/457).

The above-95% line gate passes overall and for production headers. **The above-95% branch gate FAILS**, both overall and production-only.
The report includes compiler-generated exception/unwind paths and test assertion paths; production alone has 168 uncovered branches
explicitly tagged as throw edges, plus untested logical/defensive outcomes. Unreachable examples include the SAX binary callback for JSON
text and duplicate DOM-parse error catches after successful strict SAX validation. No branches were suppressed or implementation excluded.
This is an unresolved acceptance limitation, not a coverage pass or approved exception. Remove it only with meaningful additional behavior
coverage or an explicitly agreed measurement policy; do not weaken assertions or hide branch data to obtain a percentage.

| Production header | Unsuppressed branch coverage |
| --- | --- |
| `json_bytes.ipp` | 82/116 (70.7%) |
| `json_chrono.ipp` | 146/196 (74.5%) |
| `json_impl.ipp` | 172/286 (60.1%) |
| `json_input.ipp` | 18/27 (66.7%) |
| `json_values.ipp` | 107/142 (75.4%) |
| `record_mapping.h` | 25/46 (54.3%) |
| `json.h` | 28/44 (63.6%) |

Evidence lives in `.superpowers/sdd/json-green/`: `final-smoke.log`, `accepted-format.log`, `accepted-build.log`, `accepted-tests.log`,
`accepted-coverage.log`, `accepted-production-coverage.json` and `.txt`, `sensitivity-tests.log`. HTML:
`build-json-coverage/coverage/index.html`.
Compile-time mapping validation is evidenced separately by the positive/negative compiler checks and has no runtime counters. No benchmark
claim was made. Original RED assertions were preserved; no generated build/dependency files are part of the change.

Post-implementation Constitution Check: reflection-native/backend-independent mapping, explicit policy/error contracts, focused changes,
warning-clean tests and public Doxygen gates pass. Branch coverage is the explicit unmet quality gate. Historical missing per-slice RED
observations and separate policy-only RED snapshot remain unchecked; neither is claimed retroactively.

Original user plan backup (exact bytes): `/private/tmp/rorm-json-green-original-plan-bdb40fd33ad9.md`, SHA-256
`bdb40fd33ad98f6fb70700c78286a8b523f8639b36f909e5eb3e6aa7331eed4a`. Original checkout is unchanged pending final review/delivery.
