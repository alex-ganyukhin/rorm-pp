/**
 * @file    annotations.h
 * Annotation syntax for the planned Record mapping. In `[[= expression]]`, the expression supplies a C++26 annotation value.
 * These scaffolds return tags without retaining names or converters; no consumer applies or validates them yet.
 * Intended mapping rules remain in doc/api.md until implemented.
 *
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
#ifndef RORM_PP__RORM_ANNOTATIONS_H
#define RORM_PP__RORM_ANNOTATIONS_H

#include <meta>
#include <string_view>

namespace rorm {
/**
 * @brief  Annotation type returned by rorm::name. Use the factory to create values.
 * @see    rorm::name
 */
struct name_annotation
{
    const char* data;
    std::size_t size;
};

/**
 * @brief  Annotation type returned by rorm::getter. Use the factory to create values.
 * @see    rorm::getter
 */
struct getter_annotation
{
    const char* data;
    std::size_t size;
};

/**
 * @brief  Annotation type returned by rorm::setter. Use the factory to create values.
 * @see    rorm::setter
 */
struct setter_annotation
{
    const char* data;
    std::size_t size;
};

/**
 * @brief  Annotation type returned by rorm::as. Use the factory to create values.
 * @see    rorm::as
 */
template<typename Shared, typename... Converters>
struct conversion_annotation;

template<typename Shared, typename Converter>
struct conversion_annotation<Shared, Converter>
{
    using logical_type = Shared;
    Converter first;
};

template<typename Shared, typename To, typename From>
struct conversion_annotation<Shared, To, From>
{
    using logical_type = Shared;
    To   first;
    From second;
};

/**
 * @brief  Declare a logical name shared by all mapping consumers (planned).
 *
 * Applies to public fields and enumerators. The scaffold discards the supplied name.
 *
 * @code{.cpp}
 *     struct Profile
 *     {
 *         [[= rorm::name( "displayName" )]] std::string display_name;
 *     };
 * @endcode
 */
consteval name_annotation name( std::string_view value )
{
    return name_annotation { std::define_static_string( value ), value.size() };
}

/**
 * @brief  Annotation type for rorm::ignore. Use the named constant to annotate declarations.
 * @see    rorm::ignore
 */
struct ignore_annotation
{};

/**
 * @brief  Exclude a public field from every consumer's logical properties (planned).
 *
 * @code{.cpp}
 *     struct Session
 *     {
 *         std::string                    user;
 *         [[= rorm::ignore]] std::string token; // Intended to stay outside the mapping.
 *     };
 * @endcode
 */
inline constexpr ignore_annotation ignore {};

/**
 * @brief  Declare a readable property on a public const, zero-argument method (planned).
 *
 * The argument names the logical property, independently of the method's C++ name. The scaffold discards it.
 *
 * @code{.cpp}
 *     struct Profile
 *     {
 *         [[= rorm::getter( "label" )]] std::string label() const { return "Guest"; }
 *     };
 * @endcode
 */
consteval getter_annotation getter( std::string_view value )
{
    return getter_annotation { std::define_static_string( value ), value.size() };
}

/**
 * @brief  Declare a writable property on a public one-argument method (planned).
 *
 * A getter with the same logical name forms the readable side. A setter alone is write-only. The scaffold discards the name.
 *
 * @code{.cpp}
 *     struct Profile
 *     {
 *         [[= rorm::setter( "visits" )]] void set_visits( int value ) { visits = value; }
 *         [[= rorm::ignore]] int              visits {};
 *     };
 * @endcode
 */
consteval setter_annotation setter( std::string_view value )
{
    return setter_annotation { std::define_static_string( value ), value.size() };
}

/**
 * @brief   Declare both conversion directions between a field and its logical type (planned).
 *
 * Converters return the destination value or `std::expected<destination, std::error_code>`. Returned errors are intended to pass
 * through unchanged. This scaffold retains only converter types, not their values; no conversion is performed.
 *
 * @code{.cpp}
 *     struct UserId
 *     {
 *         int value {};
 *     };
 *     struct Profile
 *     {
 *         [[= rorm::as<int>( []( UserId id ) { return id.value; },
 *                            []( int value ) { return UserId { value }; } )]] UserId id;
 *     };
 * @endcode
 *
 * @tparam  Shared  Logical type used by mapping consumers.
 */
template<typename Shared, typename ToShared, typename FromShared>
consteval auto as( ToShared to, FromShared from )
{
    return conversion_annotation<Shared, ToShared, FromShared> { to, from };
}

/**
 * @brief   Declare a getter's stored-to-logical or a setter's logical-to-stored conversion (planned).
 *
 * Uses the same converter result convention as the two-converter overload. This scaffold does not retain or invoke the converter.
 *
 * @code{.cpp}
 *     struct UserId
 *     {
 *         int value {};
 *     };
 *     struct Profile
 *     {
 *         [[= rorm::getter( "id" )]][[= rorm::as<int>( []( UserId id ) { return id.value; } )]] UserId id() const
 *         {
 *             return UserId { 42 };
 *         }
 *     };
 * @endcode
 *
 * @tparam  Shared  Logical type used by mapping consumers.
 */
template<typename Shared, typename Converter>
consteval auto as( Converter converter )
{
    return conversion_annotation<Shared, Converter> { converter };
}

/**
 * @brief  Annotation type for rorm::primary_key. Use the named constant to annotate declarations.
 * @see    rorm::primary_key
 */
struct primary_key_annotation
{};

/**
 * @brief  Identify the single primary key for ORM operations (planned).
 *
 * Applies to public fields and accessors. JSON is intended to treat this as an ordinary property.
 *
 * @code{.cpp}
 *     struct Profile
 *     {
 *         [[= rorm::primary_key]] int id {};
 *     };
 * @endcode
 */
inline constexpr primary_key_annotation primary_key {};

/**
 * @brief  Annotation type for rorm::generated. Use the named constant to annotate declarations.
 * @see    rorm::generated
 */
struct generated_annotation
{};

/**
 * @brief  Omit an ORM property from writes and retrieve its generated value (planned).
 *
 * Applies to public fields and accessors. JSON is intended to treat this as an ordinary property.
 *
 * @code{.cpp}
 *     struct Profile
 *     {
 *         [[= rorm::primary_key]][[= rorm::generated]] int id {};
 *     };
 * @endcode
 */
inline constexpr generated_annotation generated {};

/**
 * @brief  Annotation type returned by rorm::table. Use the factory to create values.
 * @see    rorm::table
 */
struct table_annotation
{
    const char* data;
    std::size_t size;
};

/**
 * @brief  Declare the table name for typed ORM operations (planned).
 *
 * Applies to a Record type. This scaffold discards the name; the PostgreSQL and ORM APIs remain provisional.
 *
 * @code{.cpp}
 *     struct[[= rorm::table( "profiles" )]] Profile
 *     {
 *         int id {};
 *     };
 * @endcode
 */
consteval table_annotation table( std::string_view value )
{
    return table_annotation { std::define_static_string( value ), value.size() };
}

/**
 * @brief  Annotation type for rorm::base64. Use the named constant to annotate declarations.
 * @see    rorm::base64
 */
struct base64_annotation
{};

/**
 * @brief  Request a base64 string representation for a byte-like property (planned).
 *
 * Applies to byte-like fields and accessors. Without this annotation, JSON is intended to use an integer array.
 *
 * @code{.cpp}
 *     struct Profile
 *     {
 *         [[= rorm::base64]] std::vector<unsigned char> avatar;
 *     };
 * @endcode
 */
inline constexpr base64_annotation base64 {};
} // namespace rorm
#endif
