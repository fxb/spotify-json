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
#include <type_traits>

#include <spotify/json/detail/macros.hpp>
#include <spotify/json/type.hpp>
#include <spotify/json/value_exception.hpp>
#include <spotify/json/value/value.hpp>

namespace spotify {
namespace json {

struct number final : public value {
 public:
  number();
  number(signed char n);
  number(signed short n);
  number(signed long n);
  number(signed long long n);
  number(signed int n);
  number(unsigned char n);
  number(unsigned short n);
  number(unsigned long n);
  number(unsigned long long n);
  number(unsigned int n);
  number(float n);
  number(double n);

  template <typename T> using number_type = typename std::enable_if<std::is_arithmetic<T>::value>::type;
  template <typename T> operator number_type<T>() const;
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

template <typename T>
number::operator number::number_type<T>() const {
  switch (_.as_value.type) {
    case detail::value_union::sint64: return T{_.as_sint64.number};
    case detail::value_union::uint64: return T{_.as_uint64.number};
    case detail::value_union::number: return T{_.as_double.number};
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
