# rorm 0.1.0 API

> Record mapping and JSON exchange below are the intended 0.1.0 contract, not implemented behavior. The headers contain API/red-test
> scaffolds: annotations discard their arguments, `to_json` returns an empty string, and `from_json` returns `Record {}` without parsing.
> PostgreSQL and typed ORM declarations remain provisional and are not present in the headers.

Implemented contracts live beside their declarations in Doxygen. This specification retains unimplemented scope until behavior is delivered,
following [ADR 0002](adr/0002-documentation-policy.md). See the [JSON quickstart](quickstart/json.md) for the intended workflow.

## Annotations

All annotations are declared in [`<rorm/annotations.h>`](../rorm/include/rorm/annotations.h). The effects below are planned.

| Annotation | Applies to | Intended effect |
| --- | --- | --- |
| `rorm::name("name")` | Public field or enumerator | Replaces the reflected name for every consumer. |
| `rorm::ignore` | Public field | Excludes the field from the logical property set. |
| `rorm::getter("name")` | Public const method with no parameters | Declares the readable side of a logical property. |
| `rorm::setter("name")` | Public method with one parameter | Declares the writable side of a logical property. |
| `rorm::as<T>(to, from)` | Public field | Converts between stored and logical types. |
| `rorm::as<T>(converter)` | Getter or setter method | Converts the accessor's available direction. |
| `rorm::primary_key` | Public field, getter, or setter | Identifies the single ORM primary key. |
| `rorm::generated` | Public field, getter, or setter | Omits ORM writes; retrieves generated values. |
| `rorm::table("name")` | Record type | Supplies the ORM table name. |
| `rorm::base64` | Byte-like field, getter, or setter | Requests base64 representation of bytes. |

## Error handling

- Invalid mappings and incompatible typed operations are intended to fail at compile time where the necessary information is available.
- Runtime operation failures use `std::expected<T, std::error_code>`; this does not promise that allocation or user code cannot throw.
- JSON reason codes and their implemented category/construction contract are documented in
  [`<rorm/json.h>`](../rorm/include/rorm/json.h). Converter-returned errors are intended to pass through unchanged.

## Planned Record mapping

A Record exposes public fields and explicitly annotated accessors as logical properties. A getter and setter with the same logical name
form one property and must agree on its logical type. Getter-only properties are readable; setter-only properties are writable. JSON reads
only readable properties on output and assigns only writable properties on input. ORM key and generated-value annotations do not exclude
properties from JSON.

Mapping validation is intended to reject duplicate logical names, field/accessor collisions, invalid accessor signatures, incompatible
getter/setter types, incompatible converters, base64 on non-byte values, and exclusion combined with other mapping annotations. No such
validation exists in the scaffolds yet.

Converters map stored values to a shared logical type and back, returning a destination value or
`std::expected<destination, std::error_code>`. Fields supply both directions; an accessor supplies its available direction.

```cpp
namespace rorm
{

consteval name_annotation name( std::string_view value );

struct ignore_annotation
{
};

inline constexpr ignore_annotation ignore {};

consteval getter_annotation getter( std::string_view property_name );
consteval setter_annotation setter( std::string_view property_name );

template<typename Shared, typename ToShared, typename FromShared>
consteval auto as( ToShared to_shared, FromShared from_shared );

template<typename Shared, typename Converter>
consteval auto as( Converter converter );

struct primary_key_annotation
{
};

struct generated_annotation
{
};

inline constexpr primary_key_annotation primary_key {};
inline constexpr generated_annotation   generated {};

consteval table_annotation table( std::string_view table_name );

struct base64_annotation
{
};

inline constexpr base64_annotation base64 {};
}
```

## JSON

The functions below are placeholders. Their Doxygen comments describe current returns and intended result handling; the following mapping
rules remain unimplemented:

- Booleans, fixed-width integers, floating-point values, and UTF-8 strings map to corresponding JSON scalar values. Reject incompatible
  scalar types, numeric overflow/underflow, and non-finite floating-point output; do not silently narrow values.
- Enums use reflected enumerator names, including `rorm::name` overrides; unknown names are incompatible values.
- Optional values use JSON null when disengaged; missing or null optional input produces a disengaged value.
- Nested Records and common sequences map recursively to objects and arrays. Fixed-size sequences require the matching input length.
- Byte sequences use integer arrays by default, or base64 strings with `rorm::base64`; malformed base64 is an incompatible value.
- `std::chrono::year_month_day` uses `YYYY-MM-DD`. Timestamps in `std::chrono::sys_time<std::chrono::milliseconds>` use ISO 8601 strings;
  normalize input offsets to UTC and emit UTC with millisecond precision.
- Deserialize a JSON object into a new Record. Require non-optional writable properties even when their C++ fields have initializers.
  Read-only mapped members may be absent and are accepted without assignment when present. Return no partial Record on failure.
- Require an explicit unknown-member policy, including for nested Records. Duplicate members fail even under `ignore`.
- Reject malformed syntax/encoding, trailing input, incompatible values, and missing required properties. JSON object order and output
  whitespace are unspecified. Return an owning string on output; do not retain the input view.

The implemented error enums and `make_error_code` are documented in [`<rorm/json.h>`](../rorm/include/rorm/json.h), rather than duplicated
here.

```cpp
namespace rorm
{
template<typename Record>
std::expected<std::string, std::error_code> to_json( const Record& record );

template<typename Record>
std::expected<Record, std::error_code> from_json( std::string_view input, unknown_members policy );
}
```

## PostgreSQL

> Provisional API spelling.

```cpp
namespace rorm::postgres
{
enum class error_code
{
    connection_failed,
    transaction_failed,
    constraint_violation,
    schema_mismatch,
    conversion_failed,
    record_not_found,
    operation_failed
};


class transaction
{
public:
    transaction( transaction&& ) noexcept;
    transaction& operator=( transaction&& ) noexcept;

    transaction( const transaction& )            = delete;
    transaction& operator=( const transaction& ) = delete;

    ~transaction();

    std::expected<void, std::error_code> commit();
    std::expected<void, std::error_code> rollback();
};

class connection
{
public:
    connection( connection&& ) noexcept;
    connection& operator=( connection&& ) noexcept;

    connection( const connection& )            = delete;
    connection& operator=( const connection& ) = delete;

    ~connection();

    std::expected<transaction, std::error_code> begin();
};

std::expected<connection, std::error_code> connect( std::string_view connection_string );

template<typename Operation>
auto atomically( connection& connection, Operation&& operation );
}
```

`atomically` begins a transaction and passes it to the callback. The callback returns `std::expected<T, std::error_code>`. On success,
`atomically` commits and returns the value. On failure, it rolls back and returns the error; a begin, commit, or rollback error is returned
if that step fails. An exception rolls back and propagates.

```cpp
auto inserted = rorm::postgres::atomically(
        connection,
        [&]( rorm::postgres::transaction& tx ) { return rorm::insert( tx, user ); } );
```

## Typed ORM

> Provisional API spelling.

```cpp
namespace rorm
{
template<typename T>
concept postgres_executor =
        std::same_as<std::remove_cvref_t<T>, postgres::connection> ||
        std::same_as<std::remove_cvref_t<T>, postgres::transaction>;

template<postgres_executor Executor, typename Record>
std::expected<Record, std::error_code> insert( Executor& executor, const Record& record );

template<postgres_executor Executor, typename Record>
std::expected<std::vector<Record>, std::error_code> insert( Executor& executor, std::span<const Record> records );

template<postgres_executor Executor, typename Record>
std::expected<Record, std::error_code> update( Executor& executor, const Record& record );

template<postgres_executor Executor, typename Record>
std::expected<std::vector<Record>, std::error_code> update( Executor& executor, std::span<const Record> records );

template<typename Record, postgres_executor Executor, typename Key>
std::expected<bool, std::error_code> erase( Executor& executor, const Key& primary_key );

template<postgres_executor Executor, typename Record>
std::expected<std::size_t, std::error_code> erase( Executor& executor, std::span<const Record> records );

template<typename Record, typename Member>
consteval auto field( Member Record::* member );

template<typename Property, typename Value>
constexpr auto operator==( Property property, Value&& value );

template<typename Property, typename Value>
constexpr auto operator!=( Property property, Value&& value );

template<typename Property, typename Value>
constexpr auto operator<( Property property, Value&& value );

template<typename Property, typename Value>
constexpr auto operator<=( Property property, Value&& value );

template<typename Property, typename Value>
constexpr auto operator>( Property property, Value&& value );

template<typename Property, typename Value>
constexpr auto operator>=( Property property, Value&& value );

template<typename Expression>
constexpr auto operator!( Expression expression );

template<typename Left, typename Right>
constexpr auto operator&&( Left left, Right right );

template<typename Left, typename Right>
constexpr auto operator||( Left left, Right right );

template<typename Property>
constexpr auto is_null( Property property );

template<typename Property>
constexpr auto is_not_null( Property property );

template<typename Property>
constexpr auto ascending( Property property );

template<typename Property>
constexpr auto descending( Property property );

template<typename Result>
class query
{
public:
    using result_type = Result;

    template<typename Expression>
    constexpr auto where( Expression expression ) const;

    template<typename Ordering>
    constexpr auto order_by( Ordering ordering ) const;

    constexpr auto limit( std::size_t value ) const;
    constexpr auto offset( std::size_t value ) const;
};

template<typename Record>
constexpr auto select();

template<typename Record, typename... Properties>
constexpr auto select( Properties... properties );

template<postgres_executor Executor, typename Query>
auto fetch( Executor& executor, const Query& query );

template<typename Record, postgres_executor Executor, typename Expression>
std::expected<std::size_t, std::error_code> count( Executor& executor, Expression expression );

template<typename Record, postgres_executor Executor, typename Expression>
std::expected<bool, std::error_code> exists( Executor& executor, Expression expression );
}
```

## Planned combined usage

This example combines intended Record/JSON behavior with provisional database APIs; it cannot run against the scaffolds.

```cpp
struct UserId
{
    std::uint64_t value {};
};

std::string user_id_to_string( UserId id );
std::expected<UserId, std::error_code> user_id_from_string( std::string_view text );

enum class UserStatus
{
    pending_review [[= rorm::name( "pending" )]],
    active,
    disabled
};

struct Address
{
    std::string city;
    std::string street;
};

struct [[= rorm::table( "users" )]] User
{
    [[= rorm::primary_key]]
    [[= rorm::generated]]
    std::int64_t id {};

    [[= rorm::as<std::string>( user_id_to_string, user_id_from_string )]]
    UserId external_id;

    [[= rorm::name( "display_name" )]]
    std::string name;

    std::optional<std::string> nickname;
    UserStatus                status {};
    Address                   address;
    std::vector<std::string>  roles;

    [[= rorm::base64]]
    std::vector<unsigned char> avatar;

    std::chrono::year_month_day                       birthday;
    std::chrono::sys_time<std::chrono::milliseconds> created_at;

    [[= rorm::getter( "email" )]]
    const std::string& email() const
    {
        return email_;
    }

    [[= rorm::setter( "email" )]]
    void set_email( std::string value )
    {
        email_ = std::move( value );
    }

    [[= rorm::getter( "display_label" )]]
    std::string display_label() const
    {
        return name + " <" + email_ + ">";
    }

private:
    std::string email_;
};

int main()
{
    auto decoded = rorm::from_json<User>(
            R"json({
                "id": 0,
                "external_id": "42",
                "display_name": "Ada",
                "nickname": null,
                "status": "active",
                "address": { "city": "Belgrade", "street": "Example Street" },
                "roles": ["admin", "author"],
                "avatar": "AQID",
                "birthday": "1990-12-10",
                "created_at": "2026-09-20T12:30:15.125+02:00",
                "email": "ada@example.com",
                "display_label": "ignored"
            })json",
            rorm::unknown_members::reject );

    if ( !decoded )
        return 1;

    auto encoded = rorm::to_json( *decoded );
    if ( !encoded )
        return 1;

    auto connected = rorm::postgres::connect( "postgresql://localhost/example" );
    if ( !connected )
        return 1;

    auto connection = std::move( *connected );
    auto started    = connection.begin();
    if ( !started )
        return 1;

    auto transaction = std::move( *started );
    auto inserted    = rorm::insert( transaction, *decoded );
    if ( !inserted )
        return 1;

    inserted->name = "Ada Lovelace";
    auto updated   = rorm::update( transaction, *inserted );
    if ( !updated )
        return 1;

    auto query =
            rorm::select<User>()
                    .where(
                            ( rorm::field( &User::status ) == UserStatus::active ) &&
                            ( rorm::field( &User::id ) > 100 ) )
                    .order_by( rorm::descending( rorm::field( &User::created_at ) ) )
                    .limit( 20 )
                    .offset( 0 );

    auto users = rorm::fetch( transaction, query );
    auto count = rorm::count<User>( transaction, rorm::field( &User::status ) == UserStatus::active );
    auto found = rorm::exists<User>( transaction, rorm::field( &User::external_id ) == UserId { 42 } );

    if ( !users || !count || !found )
        return 1;

    auto committed = transaction.commit();
    if ( !committed )
        return 1;

    auto erased = rorm::erase<User>( connection, updated->id );
    return erased ? 0 : 1;
}
```
