/*
 * Copyright (c) 2017 Spotify AB
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License. You may obtain a copy of
 * the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations under
 * the License.
 */

#pragma once

#include <cstdint>

#include <spotify/json/detail/macros.hpp>
#include <spotify/json/value/value.hpp>

namespace spotify {
namespace json {

struct number final : public value {
 public:
  number();
  number(signed char n);
  number(signed short n);
  number(signed int n);
  number(signed long n);
  number(signed long long n);
  number(unsigned char n);
  number(unsigned short n);
  number(unsigned int n);
  number(unsigned long n);
  number(unsigned long long n);
  number(float n);
  number(double n);

  operator signed char() const;
  operator signed short() const;
  operator signed int() const;
  operator signed long() const;
  operator signed long long() const;
  operator unsigned char() const;
  operator unsigned short() const;
  operator unsigned int() const;
  operator unsigned long() const;
  operator unsigned long long() const;
  operator float() const;
  operator double() const;

  template <typename T> T as() const;
};

inline number::number() : number(0) {}

inline number::number(signed char n) : value(detail::value_union::sint64) {
  _.as_sint64.number = n;
}

inline number::number(signed short n) : value(detail::value_union::sint64) {
  _.as_sint64.number = n;
}

inline number::number(signed long n) : value(detail::value_union::sint64) {
  _.as_sint64.number = n;
}

inline number::number(signed int n) : value(detail::value_union::sint64) {
  _.as_sint64.number = n;
}

inline number::number(signed long long n) : value(detail::value_union::sint64) {
  _.as_sint64.number = n;
}

inline number::number(unsigned char n) : value(detail::value_union::uint64) {
  _.as_uint64.number = n;
}

inline number::number(unsigned short n) : value(detail::value_union::uint64) {
  _.as_uint64.number = n;
}

inline number::number(unsigned long n) : value(detail::value_union::uint64) {
  _.as_uint64.number = n;
}

inline number::number(unsigned long long n) : value(detail::value_union::uint64) {
  _.as_uint64.number = n;
}

inline number::number(unsigned int n) : value(detail::value_union::uint64) {
  _.as_uint64.number = n;
}

inline number::number(float n) : value(detail::value_union::number) {
  _.as_double.number = n;
}

inline number::number(double n) : value(detail::value_union::number) {
  _.as_double.number = n;
}

inline number::operator signed char() const {
  return as<signed char>();
}

inline number::operator signed short() const {
  return as<signed short>();
}

inline number::operator signed int() const {
  return as<signed int>();
}

inline number::operator signed long() const {
  return as<signed long>();
}

inline number::operator signed long long() const {
  return as<signed long long>();
}

inline number::operator unsigned char() const {
  return as<unsigned char>();
}

inline number::operator unsigned short() const {
  return as<unsigned short>();
}

inline number::operator unsigned int() const {
  return as<unsigned int>();
}

inline number::operator unsigned long() const {
  return as<unsigned long>();
}

inline number::operator unsigned long long() const {
  return as<unsigned long long>();
}

inline number::operator float() const {
  return as<float>();
}

inline number::operator double() const {
  return as<double>();
}

template <typename T>
inline T number::as() const {
  switch (_.as_value.type) {
    case detail::value_union::sint64: return static_cast<T>(_.as_sint64.number);
    case detail::value_union::uint64: return static_cast<T>(_.as_uint64.number);
    case detail::value_union::number: return static_cast<T>(_.as_double.number);
    default: json_unreachable(); return 0;
  }
}

namespace detail {

template <>
struct construct_impl<number> {
  template <typename arg_type>
  static number construct(arg_type &&arg) {
    return number(arg);
  }
};

template <typename arg_type, typename>
number construct_impl<value>::construct(arg_type &&arg) {
  return construct_impl<number>::construct(arg);
}

}  // namespace detail
}  // namespace json
}  // namespace spotify
