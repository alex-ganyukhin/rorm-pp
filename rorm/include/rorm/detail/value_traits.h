/**
 * @file    value_traits.h
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
#include <array>
#include <chrono>
#include <cstddef>
#include <expected>
#include <optional>
#include <type_traits>
#include <vector>

namespace rorm::detail {
template<typename T>
struct optional_traits
{
    static constexpr bool value = false;
};

template<typename T>
struct optional_traits<std::optional<T>>
{
    static constexpr bool value = true;
    using element_type          = T;
};

template<typename T>
inline constexpr bool is_optional = optional_traits<T>::value;

template<typename T>
struct sequence_traits
{
    static constexpr bool value = false;
};

template<typename T, typename A>
struct sequence_traits<std::vector<T, A>>
{
    static constexpr bool value = true;
    static constexpr bool fixed = false;
    using element_type          = T;
};

template<typename T, std::size_t N>
struct sequence_traits<std::array<T, N>>
{
    static constexpr bool value = true;
    static constexpr bool fixed = true;
    static constexpr auto size  = N;
    using element_type          = T;
};

template<typename T>
inline constexpr bool is_sequence = sequence_traits<T>::value;
template<typename T>
inline constexpr bool is_byte = std::is_same_v<T, std::byte> || std::is_same_v<T, unsigned char>;

template<typename T>
consteval bool byte_sequence()
{
    if constexpr ( is_optional<T> )
        return byte_sequence<typename optional_traits<T>::element_type>();
    else if constexpr ( is_sequence<T> )
        return is_byte<typename sequence_traits<T>::element_type>;
    else
        return false;
}

template<typename T>
struct chrono_traits
{
    static constexpr bool value = false;
};

template<typename D>
struct chrono_traits<std::chrono::sys_time<D>>
{
    static constexpr bool value = true;
    using duration              = D;
};

template<typename T>
struct expected_traits
{
    static constexpr bool value = false;
    using value_type            = T;
};

template<typename T, typename E>
struct expected_traits<std::expected<T, E>>
{
    static constexpr bool value = true;
    using value_type            = T;
    using error_type            = E;
};

template<typename T>
inline constexpr bool dependent_false = false;
} // namespace rorm::detail
