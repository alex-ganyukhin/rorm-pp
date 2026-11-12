/**
 * @file    json_chrono.ipp
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

#include <algorithm>
#include <limits>

namespace rorm::detail {
// Four-digit calendar seconds and 64-bit ratio/representation products fit in 128 bits.
using chrono_integer = __int128;

inline chrono_integer chrono_floor( chrono_integer numerator, chrono_integer denominator )
{
    auto quotient = numerator / denominator;
    if ( numerator % denominator < 0 )
        --quotient;
    return quotient;
}

// Compare the borrowed decimal fraction to a rational without bounding its digit count.
inline int compare_chrono_fraction( std::string_view fraction, chrono_integer numerator, chrono_integer denominator )
{
    if ( numerator < 0 )
        return 1;
    if ( numerator >= denominator )
        return -1;
    for ( const char digit : fraction )
    {
        numerator      *= 10;
        const auto next = numerator / denominator;
        if ( digit - '0' < next )
            return -1;
        if ( digit - '0' > next )
            return 1;
        numerator %= denominator;
    }
    return numerator == 0 ? 0 : -1;
}

inline int chrono_digits( std::string_view text, std::size_t position, std::size_t length )
{
    if ( position + length > text.size() )
        return -1;
    int result = 0;
    for ( std::size_t i = position; i < position + length; ++i )
    {
        if ( text[i] < '0' || text[i] > '9' )
            return -1;
        result = result * 10 + text[i] - '0';
    }
    return result;
}

inline std::expected<std::chrono::year_month_day, std::error_code> parse_calendar( std::string_view text )
{
    const auto invalid = std::unexpected( make_error_code( json_error_code::incompatible_value ) );
    if ( text.size() != 10 || text[4] != '-' || text[7] != '-' )
        return invalid;
    const int year  = chrono_digits( text, 0, 4 );
    const int month = chrono_digits( text, 5, 2 );
    const int day   = chrono_digits( text, 8, 2 );
    if ( year < 0 || month < 0 || day < 0 )
        return invalid;
    const std::chrono::year_month_day result { std::chrono::year( year ),
                                               std::chrono::month( month ),
                                               std::chrono::day( day ) };
    if ( !result.ok() )
        return invalid;
    return result;
}

inline void append_chrono_digits( std::string& output, unsigned value, unsigned width )
{
    const auto start = output.size();
    output.append( width, '0' );
    while ( width != 0 )
    {
        output[start + --width] = static_cast<char>( '0' + value % 10 );
        value                  /= 10;
    }
}

inline std::string format_calendar( std::chrono::year_month_day date )
{
    std::string result;
    append_chrono_digits( result, static_cast<unsigned>( static_cast<int>( date.year() ) ), 4 );
    result += '-';
    append_chrono_digits( result, static_cast<unsigned>( date.month() ), 2 );
    result += '-';
    append_chrono_digits( result, static_cast<unsigned>( date.day() ), 2 );
    return result;
}

template<typename Chrono>
std::expected<std::string, std::error_code> format_chrono( const Chrono& value )
{
    const auto invalid = std::unexpected( make_error_code( json_error_code::incompatible_value ) );
    const auto range   = std::unexpected( make_error_code( json_error_code::out_of_range ) );
    if constexpr ( std::is_same_v<Chrono, std::chrono::year_month_day> )
    {
        if ( !value.ok() || int( value.year() ) < 0 || int( value.year() ) > 9999 )
            return invalid;
        return format_calendar( value );
    }
    else
    {
        using D   = typename chrono_traits<Chrono>::duration;
        using Rep = typename D::rep;
        static_assert( std::is_integral_v<Rep> && sizeof( Rep ) <= 8,
                       "JSON timestamps require an integral representation of at most 64 bits; use rorm::as" );
        using Period = typename D::period;
        static_assert( Period::num > 0 && Period::den > 0, "JSON timestamp periods must be positive" );
        constexpr chrono_integer numerator   = Period::num;
        constexpr chrono_integer denominator = Period::den;
        const chrono_integer     total   = static_cast<chrono_integer>( value.time_since_epoch().count() ) * numerator;
        const auto               seconds = chrono_floor( total, denominator );
        using namespace std::chrono;
        constexpr auto minimum =
                duration_cast<std::chrono::seconds>( sys_days( year( 0 ) / January / 1 ).time_since_epoch() ).count();
        constexpr auto maximum =
                duration_cast<std::chrono::seconds>( sys_days( year( 10000 ) / January / 1 ).time_since_epoch() ).count();
        if ( seconds < minimum || seconds >= maximum )
            return range;
        const sys_seconds instant( std::chrono::seconds( static_cast<long long>( seconds ) ) );
        const auto        day = floor<days>( instant );
        const hh_mm_ss    time( instant - day );
        auto              result = format_calendar( year_month_day( day ) );
        result                  += 'T';
        append_chrono_digits( result, time.hours().count(), 2 );
        result += ':';
        append_chrono_digits( result, time.minutes().count(), 2 );
        result += ':';
        append_chrono_digits( result, time.seconds().count(), 2 );
        auto     remaining_denominator = denominator;
        unsigned twos                  = 0;
        unsigned fives                 = 0;
        while ( remaining_denominator % 2 == 0 )
        {
            remaining_denominator /= 2;
            ++twos;
        }
        while ( remaining_denominator % 5 == 0 )
        {
            remaining_denominator /= 5;
            ++fives;
        }
        // Repeating periods use 20 fractional digits: truncation is below half the smallest supported tick.
        const unsigned width = remaining_denominator == 1 ? std::max( twos, fives ) : 20;
        if ( width != 0 )
        {
            result        += '.';
            auto remainder = total - seconds * denominator;
            for ( unsigned i = 0; i < width; ++i )
            {
                remainder *= 10;
                result    += static_cast<char>( '0' + remainder / denominator );
                remainder %= denominator;
            }
        }
        result += 'Z';
        return result;
    }
}

template<typename Chrono>
std::expected<Chrono, std::error_code> parse_chrono( std::string_view text )
{
    if constexpr ( std::is_same_v<Chrono, std::chrono::year_month_day> )
        return parse_calendar( text );
    else
    {
        const auto invalid = std::unexpected( make_error_code( json_error_code::incompatible_value ) );
        const auto range   = std::unexpected( make_error_code( json_error_code::out_of_range ) );
        using D            = typename chrono_traits<Chrono>::duration;
        using Rep          = typename D::rep;
        using Period       = typename D::period;
        static_assert( std::is_integral_v<Rep> && sizeof( Rep ) <= 8,
                       "JSON timestamps require an integral representation of at most 64 bits; use rorm::as" );
        static_assert( Period::num > 0 && Period::den > 0, "JSON timestamp periods must be positive" );
        if ( text.size() < 20 || text[10] != 'T' || text[13] != ':' || text[16] != ':' )
            return invalid;
        const auto date = parse_calendar( text.substr( 0, 10 ) );
        if ( !date )
            return invalid;
        const int hour   = chrono_digits( text, 11, 2 );
        const int minute = chrono_digits( text, 14, 2 );
        const int second = chrono_digits( text, 17, 2 );
        if ( hour < 0 || hour > 23 || minute < 0 || minute > 59 || second < 0 || second > 59 )
            return invalid;
        std::size_t      position = 19;
        std::string_view fraction;
        if ( text[position] == '.' )
        {
            const auto start = ++position;
            while ( position < text.size() && text[position] >= '0' && text[position] <= '9' )
                ++position;
            if ( start == position )
                return invalid;
            fraction = text.substr( start, position - start );
        }
        int offset = 0;
        if ( position < text.size() && text[position] == 'Z' )
            ++position;
        else if ( position + 6 == text.size() && ( text[position] == '+' || text[position] == '-' ) &&
                  text[position + 3] == ':' )
        {
            const int offset_hour   = chrono_digits( text, position + 1, 2 );
            const int offset_minute = chrono_digits( text, position + 4, 2 );
            if ( offset_hour < 0 || offset_hour > 23 || offset_minute < 0 || offset_minute > 59 )
                return invalid;
            offset    = ( offset_hour * 60 + offset_minute ) * 60 * ( text[position] == '+' ? 1 : -1 );
            position += 6;
        }
        else
            return invalid;
        if ( position != text.size() )
            return invalid;
        using namespace std::chrono;
        const chrono_integer seconds =
                duration_cast<std::chrono::seconds>( sys_days( *date ).time_since_epoch() ).count() + hour * 3600 +
                minute * 60 + second - offset;
        constexpr auto minimum =
                duration_cast<std::chrono::seconds>( sys_days( year( 0 ) / January / 1 ).time_since_epoch() ).count();
        constexpr auto maximum =
                duration_cast<std::chrono::seconds>( sys_days( year( 10000 ) / January / 1 ).time_since_epoch() ).count();
        if ( seconds < minimum || seconds >= maximum )
            return range;
        constexpr chrono_integer numerator      = Period::num;
        constexpr chrono_integer denominator    = Period::den;
        const auto               scaled_seconds = seconds * denominator;
        auto                     lower          = chrono_floor( scaled_seconds, numerator );
        auto                     upper          = chrono_floor( ( seconds + 1 ) * denominator, numerator ) + 1;
        while ( lower + 1 < upper )
        {
            const auto middle = lower + ( upper - lower ) / 2;
            if ( compare_chrono_fraction( fraction, middle * numerator - scaled_seconds, denominator ) >= 0 )
                lower = middle;
            else
                upper = middle;
        }
        const int midpoint =
                compare_chrono_fraction( fraction, ( 2 * lower + 1 ) * numerator - 2 * scaled_seconds, 2 * denominator );
        if ( midpoint > 0 || ( midpoint == 0 && lower % 2 != 0 ) )
            ++lower;
        if ( lower < static_cast<chrono_integer>( std::numeric_limits<Rep>::min() ) ||
             lower > static_cast<chrono_integer>( std::numeric_limits<Rep>::max() ) )
            return range;
        const auto rounded_seconds = chrono_floor( lower * numerator, denominator );
        if ( rounded_seconds < minimum || rounded_seconds >= maximum )
            return range;
        return Chrono( D( static_cast<Rep>( lower ) ) );
    }
}
} // namespace rorm::detail
