#ifndef RORM_PP__TESTS_EXPAND_H
#define RORM_PP__TESTS_EXPAND_H

#include <experimental/meta>
#include <vector>

// Workaround from https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/p2996r9.html#implementation-status

namespace __impl {
template<auto... vals>
struct replicator_type
{
    template<typename F>
    constexpr void operator>>( F body ) const
    {
        ( body.template operator()<vals>(), ... );
    }
};

template<auto... vals>
replicator_type<vals...> replicator = {};
} // namespace __impl

template<typename R>
consteval auto expand( R range )
{
    std::vector<std::meta::info> args;
    for ( auto r : range )
    {
        args.push_back( reflect_value( r ) );
    }
    return substitute( ^^__impl::replicator, args );
}

#endif /* RORM_PP__TESTS_EXPAND_H */
