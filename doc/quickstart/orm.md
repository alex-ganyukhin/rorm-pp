# ORM quickstart

> Provisional 0.1.0 API; not yet implemented.

## Scenario: create and manage a task

Map a C++ type to an existing PostgreSQL table. The table needs `id` as a generated primary key and `title` as a text column. Connect, insert a task, find it, update it, and delete it within one transaction:

```cpp
struct [[= rorm::table( "tasks" )]] Task
{
    [[= rorm::primary_key]]
    [[= rorm::generated]]
    std::int64_t id {};

    std::string title;
};

int main()
{
    auto connected = rorm::postgres::connect( "postgresql://localhost/example" );
    if ( !connected )
        return 1; // Inspect connected.error().

    auto connection = std::move( *connected );
    auto result = rorm::postgres::atomically(
            connection,
            []( rorm::postgres::transaction& tx ) -> std::expected<void, rorm::postgres::error> {
                auto inserted = rorm::insert( tx, Task { .title = "Write report" } );
                if ( !inserted )
                    return std::unexpected( inserted.error() );
                // inserted->id contains the database-generated key.

                auto tasks = rorm::fetch(
                        tx,
                        rorm::select<Task>().where( rorm::field( &Task::id ) == inserted->id ) );
                if ( !tasks )
                    return std::unexpected( tasks.error() );

                inserted->title = "Send report";
                auto updated    = rorm::update( tx, *inserted );
                if ( !updated )
                    return std::unexpected( updated.error() );

                auto erased = rorm::erase<Task>( tx, updated->id );
                if ( !erased )
                    return std::unexpected( erased.error() );

                return {};
            } );
    return result ? 0 : 1;
}
```

`atomically` commits on success and rolls back on error. `insert` and `update` return new Record values; they do not change the input object.

Extended features:

- Batch insert/update/delete: Process multiple Records.
- Comparisons: Filter by field values.
- Null predicates: Check nullable fields.
- Combined filters: Compose conditions.
- Sorting: Order query results.
- Limit/offset: Page through results.
- Selected properties: Fetch specific fields.
- `count`: Count matching Records.
- `exists`: Check for matches.
- Manual transactions: Control commit and rollback.
