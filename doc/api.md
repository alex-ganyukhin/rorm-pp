# rorm 0.1.0 API

> Record and JSON declarations are the 0.1.0 contract. PostgreSQL and typed ORM declarations remain provisional until their milestones.

## Annotations

| Annotation               | Applies to                             | Effect                                                            |
| ------------------------ | -------------------------------------- | ----------------------------------------------------------------- |
| `rorm::name("name")`     | Public field or enumerator             | Replaces the reflected name for every consumer.                   |
| `rorm::ignore`           | Public field                           | Excludes the field from the logical property set.                 |
| `rorm::getter("name")`   | Public const method with no parameters | Declares the readable side of a logical property.                 |
| `rorm::setter("name")`   | Public method with one parameter       | Declares the writable side of a logical property.                 |
| `rorm::as<T>(to, from)`  | Public field                           | Converts between the stored and logical types.                    |
| `rorm::as<T>(converter)` | Getter or setter method                | Converts the accessor's available direction.                      |
| `rorm::primary_key`      | Public field, getter, or setter        | Identifies the single primary key used by ORM operations.         |
| `rorm::generated`        | Public field, getter, or setter        | Omits the property from writes and retrieves its generated value. |
| `rorm::table("name")`    | Record type                            | Supplies the table name to typed ORM operations.                  |
| `rorm::json::base64`     | Byte-like field, getter, or setter     | Uses a base64 JSON string instead of an integer array.            |

## Error handling

- As much errors as possible are reported at a compile-time:
  - Consistency of the definitions, including missing setters or getters for reading or writing operations
  - Correct usage of annotations
  - Correct usage of transactions, connections, and ORM operations
  - ...
- The records which cannot be detected at a compile-time will be reported at runtime via std::expected<X, std::error_code>.
- The library aims at throwing exceptions only for unrecoverable errors and hard faults, which hopefully remain rare.

```cpp
namespace rorm
{
class conversion_error
{
public:
    std::string_view message() const noexcept;
};

consteval auto name( std::string_view value );

struct ignore_annotation
{
};

inline constexpr ignore_annotation ignore {};

consteval auto getter( std::string_view property_name );
consteval auto setter( std::string_view property_name );

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

consteval auto table( std::string_view table_name );

namespace json
{
struct base64_annotation
{
};

inline constexpr base64_annotation base64 {};
}
}
```

## JSON

```cpp
namespace rorm
{
enum class unknown_members
{
    reject,
    ignore
};

enum class json_error_code
{
    malformed_input,
    duplicate_member,
    unknown_member,
    missing_property,
    incompatible_value,
    out_of_range,
    conversion_failed
};

class json_error
{
public:
    json_error_code            code() const noexcept;
    std::string_view           path() const noexcept;
    std::optional<std::size_t> byte_offset() const noexcept;
    std::string_view           message() const noexcept;
};

template<typename Record>
std::expected<std::string, json_error> to_json( const Record& record );

template<typename Record>
std::expected<Record, json_error> from_json( std::string_view input, unknown_members policy );
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

class error
{
public:
    error_code       code() const noexcept;
    std::string_view message() const noexcept;
};

class transaction
{
public:
    transaction( transaction&& ) noexcept;
    transaction& operator=( transaction&& ) noexcept;

    transaction( const transaction& )            = delete;
    transaction& operator=( const transaction& ) = delete;

    ~transaction();

    std::expected<void, error> commit();
    std::expected<void, error> rollback();
};

class connection
{
public:
    connection( connection&& ) noexcept;
    connection& operator=( connection&& ) noexcept;

    connection( const connection& )            = delete;
    connection& operator=( const connection& ) = delete;

    ~connection();

    std::expected<transaction, error> begin();
};

std::expected<connection, error> connect( std::string_view connection_string );

template<typename Operation>
auto atomically( connection& connection, Operation&& operation );
}
```

`atomically` begins a transaction and passes it to the callback. The callback returns `std::expected<T, error>`. On success, `atomically` commits and returns the value. On failure, it rolls back and returns the error; a begin, commit, or rollback error is returned if that step fails. An exception rolls back and propagates.

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
std::expected<Record, postgres::error> insert( Executor& executor, const Record& record );

template<postgres_executor Executor, typename Record>
std::expected<std::vector<Record>, postgres::error> insert( Executor& executor, std::span<const Record> records );

template<postgres_executor Executor, typename Record>
std::expected<Record, postgres::error> update( Executor& executor, const Record& record );

template<postgres_executor Executor, typename Record>
std::expected<std::vector<Record>, postgres::error> update( Executor& executor, std::span<const Record> records );

template<typename Record, postgres_executor Executor, typename Key>
std::expected<bool, postgres::error> erase( Executor& executor, const Key& primary_key );

template<postgres_executor Executor, typename Record>
std::expected<std::size_t, postgres::error> erase( Executor& executor, std::span<const Record> records );

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
std::expected<std::size_t, postgres::error> count( Executor& executor, Expression expression );

template<typename Record, postgres_executor Executor, typename Expression>
std::expected<bool, postgres::error> exists( Executor& executor, Expression expression );
}
```

## Usage

```cpp
struct UserId
{
    std::uint64_t value {};
};

std::string user_id_to_string( UserId id );
std::expected<UserId, rorm::conversion_error> user_id_from_string( std::string_view text );

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

    [[= rorm::json::base64]]
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
