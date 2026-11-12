# rorm 0.1.0 API

Record mapping and JSON exchange are implemented. Their primary contracts and local examples live in
[`<rorm/annotations.h>`](../rorm/include/rorm/annotations.h) and [`<rorm/json.h>`](../rorm/include/rorm/json.h), following
[ADR 0002](adr/0002-documentation-policy.md). Start with the [JSON quickstart](quickstart/json.md) for a practical workflow.

The shared reflection mapping keeps property names, directionality and conversions independent of the JSON backend; see
[ADR 0001](adr/0001-use-nlohmann-json-for-initial-json-support.md) for the initial backend decision. PostgreSQL and typed ORM declarations
below remain provisional and are not present in the headers.

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

This example combines implemented Record/JSON behavior with provisional database APIs; the database portion cannot run yet.

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
            {} );

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
