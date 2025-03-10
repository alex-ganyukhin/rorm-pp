/**
 * Copyright 2025 Aleksandr Ganiukhin
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
 *
 * @file     expand.h
 * @date     2025-03-10
 *
 * @author   Alexander Ganyukhin (aganyukhin@outlook.com)
 *
 * @brief
 */

#ifndef RORM_PP__RORM_EXPAND_H
#define RORM_PP__RORM_EXPAND_H

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

#endif /* RORM_PP__RORM_EXPAND_H */
