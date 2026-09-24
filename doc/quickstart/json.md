# JSON quickstart

> [!NOTE]
> This describes the planned 0.1.0 Record and JSON API.

## Scenario: accept and return a profile

Define an ordinary C++ type. Its public fields become JSON properties; no registration or base class is needed. Parse an incoming profile, change it, and serialize the result:

```cpp
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
        return 1; // Inspect profile.error() for code, path, and message.

    ++profile->visits;
    auto response = rorm::to_json( *profile );
    if ( !response )
        return 1; // Inspect response.error().

    // *response is a JSON string containing the updated profile.
}
```

The unknown-member policy is required on every `from_json` call. Use `reject` to catch unexpected input or `ignore` when extra members are acceptable. Missing writable properties are allowed only when they are optional.

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
- `rorm::json::base64`: Encode bytes as a string.
- JSON errors: Inspect code, path, and message.
