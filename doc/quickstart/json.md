# JSON quickstart

> [!NOTE]
> The Record and JSON interfaces are present as dummy scaffolds for the API/red-test phase.
> `to_json` returns an empty string and `from_json` returns a default object. They do not exchange JSON yet.
> The workflow and features below describe intended behavior.

## Scenario: accept and return a profile

Define an ordinary C++ type. Its public fields become JSON properties; no registration or base class is needed. Parse an incoming profile,
change it, and serialize the result. This example illustrates the planned workflow:

```cpp
#include <rorm/rorm.h>

#include <optional>
#include <string>

struct Profile
{
    std::string                name;
    std::optional<std::string> nickname;
    int                        visits {};
};

int main()
{
    auto profile = rorm::from_json<Profile>(
            R"({"name":"Ada","nickname":null,"visits":1})",
            rorm::unknown_members::reject );
    if ( !profile )
        return 1; // Inspect profile.error() for the error code and its category message.

    ++profile->visits;
    auto response = rorm::to_json( *profile );
    if ( !response )
        return 1; // Inspect response.error().

    // *response is a JSON string containing the updated profile.
}
```

The unknown-member policy is required on every `from_json` call. Use `reject` to catch unexpected input or `ignore` when extra members are
acceptable. Missing writable properties are allowed only when they are optional.

Extended features:

- `rorm::name`: Rename a property or enumerator.
- `rorm::ignore`: Exclude a field.
- `rorm::getter` / `rorm::setter`: Map accessor methods.
- `rorm::as`: Convert stored and logical values.
- Nested Records: Serialize nested objects.
- Sequences: Serialize arrays.
- Enums: Serialize enumerator names.
- Dates and times: Serialize supported chrono values.
- Byte arrays: Serialize integer arrays.
- `rorm::base64`: Encode bytes as a string.
- JSON errors: Inspect the error code and its category message.

See the [planned mapping contract](../api.md#planned-record-mapping) and the local API examples in
[`<rorm/annotations.h>`](../../rorm/include/rorm/annotations.h) and [`<rorm/json.h>`](../../rorm/include/rorm/json.h).
