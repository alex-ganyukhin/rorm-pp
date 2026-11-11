/**
 * @file    record_mapping.h
 * Copyright 2026 Aleksandr Ganiukhin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once
#include <functional>
#include <meta>
#include <rorm/annotations.h>
#include <rorm/detail/value_traits.h>
#include <string_view>
#include <system_error>

namespace rorm::detail {
template<typename T>
struct conversion_traits
{
    static constexpr bool value = false;
};

template<typename S, typename... C>
struct conversion_traits<conversion_annotation<S, C...>>
{
    static constexpr bool value = true;
    using logical_type          = S;
    static constexpr auto count = sizeof...( C );
};

template<typename A>
consteval std::size_t annotation_count( std::meta::info m )
{
    std::size_t n = 0;
    for ( auto a : std::meta::annotations_of( m ) )
        if ( std::meta::remove_cv( std::meta::type_of( a ) ) == ^^A )
            ++n;
    return n;
}

template<typename A>
consteval bool has_annotation( std::meta::info m )
{
    for ( auto a : std::meta::annotations_of( m ) )
        if ( std::meta::remove_cv( std::meta::type_of( a ) ) == ^^A )
            return true;
    return false;
}

template<typename A>
consteval A annotation( std::meta::info m )
{
    for ( auto a : std::meta::annotations_of( m ) )
        if ( std::meta::remove_cv( std::meta::type_of( a ) ) == ^^A )
            return std::meta::extract<A>( a );
    throw "missing annotation";
}

consteval std::meta::info conversion_of( std::meta::info m )
{
    for ( auto a : std::meta::annotations_of( m ) )
    {
        auto t = std::meta::type_of( a );
        if ( std::meta::has_template_arguments( t ) && std::meta::template_of( t ) == ^^conversion_annotation )
            return a;
    }
    return {};
}

consteval std::string_view logical_name( std::meta::info m )
{
    if ( has_annotation<name_annotation>( m ) )
    {
        auto a = annotation<name_annotation>( m );
        return { a.data, a.size };
    }
    if ( has_annotation<getter_annotation>( m ) )
    {
        auto a = annotation<getter_annotation>( m );
        return { a.data, a.size };
    }
    if ( has_annotation<setter_annotation>( m ) )
    {
        auto a = annotation<setter_annotation>( m );
        return { a.data, a.size };
    }
    return std::meta::identifier_of( m );
}

/** @brief Compile-time metadata only; reflections must never escape into runtime storage. */
struct property_descriptor
{
    std::string_view name;
    std::meta::info  reader;
    std::meta::info  writer;
    bool             base64;
    bool             primary_key;
    bool             generated;
};

/** @brief Owning normalized property operations; converter errors retain their original category and value. */
template<typename Record, std::meta::info M>
struct member_mapping
{
    static constexpr bool field      = std::meta::is_nonstatic_data_member( M );
    static constexpr bool get        = has_annotation<getter_annotation>( M );
    static constexpr auto conversion = conversion_of( M );

    static consteval auto stored_info()
    {
        if constexpr ( field )
            return std::meta::type_of( M );
        else if constexpr ( get )
            return std::meta::return_type_of( M );
        else
            return std::meta::type_of( std::meta::parameters_of( M )[0] );
    }

    using stored_type = std::remove_cvref_t<typename[:stored_info():]>;

    static consteval auto logical_info()
    {
        if constexpr ( conversion != std::meta::info {} )
        {
            using C = std::remove_cv_t<typename[:std::meta::type_of( conversion ):]>;
            return ^^typename C::logical_type;
        }
        else
            return ^^stored_type;
    }

    using logical_type = std::remove_cvref_t<typename[:logical_info():]>;

    template<typename Result, typename Dest>
    static consteval bool valid_result()
    {
        if constexpr ( expected_traits<Result>::value )
            return std::is_same_v<typename expected_traits<Result>::error_type, std::error_code> &&
                   std::is_constructible_v<Dest, typename expected_traits<Result>::value_type>;
        else
            return std::is_constructible_v<Dest, Result>;
    }

    template<typename F, typename Arg, typename Dest>
    static consteval bool valid_converter()
    {
        if constexpr ( std::is_invocable_v<F, Arg> )
            return valid_result<std::invoke_result_t<F, Arg>, Dest>();
        else
            return false;
    }

    static consteval bool validate()
    {
        if constexpr ( !field )
        {
            constexpr auto fn = &[:M:];
            if constexpr ( get )
                static_assert( std::is_invocable_v<decltype( fn ), const Record&>,
                               "getter must be callable on const Record" );
            else
                static_assert( std::is_invocable_v<decltype( fn ), Record&, stored_type>,
                               "setter must accept its normalized value" );
        }

        if constexpr ( conversion != std::meta::info {} )
        {
            using C       = std::remove_cv_t<typename[:std::meta::type_of( conversion ):]>;
            constexpr C c = std::meta::extract<C>( conversion );
            if constexpr ( field )
            {
                static_assert( conversion_traits<C>::count == 2, "convert field requires both directions" );
                if constexpr ( conversion_traits<C>::count == 2 )
                {
                    static_assert( valid_converter<decltype( c.first ), stored_type, logical_type>(),
                                   "convert to logical type" );
                    static_assert( valid_converter<decltype( c.second ), logical_type, stored_type>(),
                                   "convert to stored type" );
                }
            }
            else if constexpr ( get )
                static_assert( valid_converter<decltype( c.first ), stored_type, logical_type>(), "convert getter" );
            else
                static_assert( valid_converter<decltype( c.first ), logical_type, stored_type>(), "convert setter" );
        }
        return true;
    }

    template<typename Dest, typename V>
    static std::expected<Dest, std::error_code> normalize( V&& v )
    {
        if constexpr ( expected_traits<std::remove_cvref_t<V>>::value )
        {
            if ( !v )
                return std::unexpected( v.error() );
            return Dest( std::move( *v ) );
        }
        else
            return Dest( std::forward<V>( v ) );
    }

    static std::expected<logical_type, std::error_code> read( const Record& r )
    {
        auto value = [&]() -> stored_type {
            if constexpr ( field )
                return r.[:M:];
            else
            {
                constexpr auto fn = &[:M:];
                return std::invoke( fn, r );
            }
        }();
        if constexpr ( conversion != std::meta::info {} )
        {
            using C       = std::remove_cv_t<typename[:std::meta::type_of( conversion ):]>;
            constexpr C c = std::meta::extract<C>( conversion );
            return normalize<logical_type>( std::invoke( c.first, std::move( value ) ) );
        }
        else
            return value;
    }

    static std::expected<void, std::error_code> write( Record& r, logical_type v )
    {
        auto assign = [&]( stored_type value ) {
            if constexpr ( field )
                r.[:M:] = std::move( value );
            else
            {
                constexpr auto fn = &[:M:];
                std::invoke( fn, r, std::move( value ) );
            }
        };
        if constexpr ( conversion != std::meta::info {} )
        {
            using C               = std::remove_cv_t<typename[:std::meta::type_of( conversion ):]>;
            constexpr C c         = std::meta::extract<C>( conversion );
            auto        converted = [&]() {
                if constexpr ( field )
                    return normalize<stored_type>( std::invoke( c.second, std::move( v ) ) );
                else
                    return normalize<stored_type>( std::invoke( c.first, std::move( v ) ) );
            }();
            if ( !converted )
                return std::unexpected( converted.error() );
            assign( std::move( *converted ) );
        }
        else
            assign( std::move( v ) );
        return {};
    }
};

template<typename R, std::meta::info Reader, std::meta::info Writer>
struct mapped_property
{
    static constexpr bool readable = Reader != std::meta::info {};
    static constexpr bool writable = Writer != std::meta::info {};
    using logical_type             = typename member_mapping<R, readable ? Reader : Writer>::logical_type;

    static std::expected<logical_type, std::error_code> read( const R& r )
        requires( readable )
    {
        return member_mapping<R, Reader>::read( r );
    }

    static std::expected<void, std::error_code> write( R& r, logical_type v )
        requires( writable )
    {
        return member_mapping<R, Writer>::write( r, std::move( v ) );
    }
};

template<typename Record>
struct record_mapping
{
    static consteval auto discover()
    {
        std::vector<property_descriptor> result;
        template for ( constexpr auto m : std::define_static_array(
                               std::meta::members_of( ^^Record, std::meta::access_context::current() ) ) )
        {
            if constexpr ( std::meta::is_public( m ) &&
                           ( std::meta::is_nonstatic_data_member( m ) || std::meta::is_function( m ) ) )
            {
                constexpr bool field = std::meta::is_nonstatic_data_member( m );
                constexpr bool get   = has_annotation<getter_annotation>( m );
                constexpr bool set   = has_annotation<setter_annotation>( m );
                if constexpr ( field || get || set )
                {
                    static_assert( annotation_count<name_annotation>( m ) <= 1 &&
                                           annotation_count<getter_annotation>( m ) <= 1 &&
                                           annotation_count<setter_annotation>( m ) <= 1,
                                   "duplicate mapping annotation" );
                    static_assert( !( get && set ), "getter and setter annotations conflict" );
                    if constexpr ( has_annotation<ignore_annotation>( m ) )
                    {
                        static_assert( !has_annotation<name_annotation>( m ) && !get && !set &&
                                               conversion_of( m ) == std::meta::info {} &&
                                               !has_annotation<base64_annotation>( m ),
                                       "ignore conflicts with mapping annotations" );
                    }
                    else
                    {
                        if constexpr ( get )
                        {
                            static_assert( !std::meta::is_static_member( m ) && std::meta::is_const( m ) &&
                                                   std::meta::parameters_of( m ).empty() &&
                                                   std::meta::return_type_of( m ) != ^^void,
                                           "getter must be const and take no arguments" );
                        }
                        if constexpr ( set )
                        {
                            static_assert( !std::meta::is_static_member( m ) && std::meta::parameters_of( m ).size() == 1,
                                           "setter requires exactly one argument" );
                        }
                        if constexpr ( ( !get || (std::meta::is_const( m ) && std::meta::parameters_of( m ).empty() &&
                                                  std::meta::return_type_of( m ) != ^^void)) &&
                                       ( !set || std::meta::parameters_of( m ).size() == 1 ) )
                        {
                            using MP = member_mapping<Record, m>;
                            static_assert( MP::validate() );
                            static_assert( !has_annotation<base64_annotation>( m ) ||
                                                   byte_sequence<typename MP::logical_type>(),
                                           "base64 requires logical byte sequence" );
                            property_descriptor p { logical_name( m ),
                                                    field || get ? m : std::meta::info {},
                                                    field || set ? m : std::meta::info {},
                                                    has_annotation<base64_annotation>( m ),
                                                    has_annotation<primary_key_annotation>( m ),
                                                    has_annotation<generated_annotation>( m ) };
                            bool                found = false;
                            for ( auto& prior : result )
                                if ( prior.name == p.name )
                                {
                                    if ( ( prior.reader != std::meta::info {} && p.reader != std::meta::info {} ) ||
                                         ( prior.writer != std::meta::info {} && p.writer != std::meta::info {} ) )
                                        throw "duplicate logical property";
                                    prior.reader       = prior.reader != std::meta::info {} ? prior.reader : p.reader;
                                    prior.writer       = prior.writer != std::meta::info {} ? prior.writer : p.writer;
                                    prior.base64      |= p.base64;
                                    prior.primary_key |= p.primary_key;
                                    prior.generated   |= p.generated;
                                    found              = true;
                                }
                            if ( !found )
                                result.push_back( p );
                        }
                    }
                }
            }
        }
        return result;
    }

    static consteval auto materialize()
    {
        std::array<property_descriptor, discover().size()> result {};
        auto                                               source = discover();
        if constexpr ( result.size() > 0 )
            for ( std::size_t i = 0; i < source.size(); ++i )
                result[i] = source[i];
        return result;
    }

    static constexpr auto properties = materialize();

    static consteval bool validate_pairs()
    {
        template for ( constexpr auto p : properties )
        {
            if constexpr ( p.reader != std::meta::info {} && p.writer != std::meta::info {} )
                static_assert( std::is_same_v<typename member_mapping<Record, p.reader>::logical_type,
                                              typename member_mapping<Record, p.writer>::logical_type>,
                               "accessor logical type mismatch" );
        }
        return true;
    }

    static_assert( validate_pairs() );
};
} // namespace rorm::detail
