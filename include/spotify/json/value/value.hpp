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

#include <utility>
#include <tuple>
#include <type_traits>

#include <spotify/json/encoded_value.hpp>
#include <spotify/json/type.hpp>
#include <spotify/json/value/construct.hpp>
#include <spotify/json/value/detail.hpp>

namespace spotify {
namespace json {
namespace detail {

template <typename value_type> struct cast_impl;

template <class T>
using decay_t = typename std::decay<T>::type;

template <bool B, class T = void>
using enable_if_t = typename std::enable_if<B, T>::type;

}  // namespace detail

struct boolean;
struct number;
struct string;

struct value {
 public:
  value();

  // Enable only for non-base types of 'value'.
  template <typename T0, typename ...Ts, typename = detail::enable_if_t<!std::is_base_of<value, detail::decay_t<T0>>::value>>
  explicit value(T0 arg, Ts &&...args);

  // Enable only for non-base types of 'value'.
  template <typename T, typename = detail::enable_if_t<!std::is_base_of<value, detail::decay_t<T>>::value>>
  value &operator=(T &&v);

  type type() const;

 protected:
  template <typename value_type> friend struct detail::cast_impl;

  value(detail::value_union::type type) : _(type) {}
  detail::value_union _;
};

static_assert(sizeof(value) == sizeof(detail::value_union), "size of value should equal size of value_union");

template <typename T0, typename ...Ts, typename>
inline value::value(T0 arg, Ts &&...args) {
  *this = detail::construct<value>(std::forward<T0>(arg), std::forward<Ts>(args)...);
}

template <typename T, typename>
inline value &value::operator=(T &&v) {
  *this = detail::construct<value>(std::forward<T>(v));
  return *this;
}

inline value::value() : value(detail::value_union::value_null) {}

inline type value::type() const {
  switch (_.as_value.type) {
    case detail::value_union::string: return type::string;
    case detail::value_union::object: return type::object;
    case detail::value_union::array:  return type::array;
    case detail::value_union::sint64: return type::number;
    case detail::value_union::uint64: return type::number;
    case detail::value_union::number: return type::number;
    case detail::value_union::value_false: return type::boolean;
    case detail::value_union::value_true:  return type::boolean;
    case detail::value_union::value_null:  return type::null;
    default: return type::string;  // case ...::type::short_string_xx
  }
}

namespace detail {

template <class T>
struct is_boolean : std::is_same<decay_t<T>, bool> {};

template <class T>
struct is_number : std::integral_constant<bool, std::is_arithmetic<T>::value && !is_boolean<T>::value> {};

template <typename T, bool B, class T0, class ...Ts>
using enable_construct_t = enable_if_t<B && std::is_constructible<T, T0, Ts...>::value && !std::is_same<T, decay_t<T0>>::value>;

template <>
struct construct_impl<value> {
  // Enable only for bool type.
  template <typename T, typename = enable_construct_t<boolean, is_boolean<T>::value, T>>
  static boolean construct(T &&v);

  // Enable only for arithmetic types (and not bool).
  template <typename T, typename = enable_construct_t<number, is_number<T>::value, T>>
  static number construct(T &&v);

  // Enable only for arguments 'string' is constructible with.
  template <typename T0, typename ...Ts, typename = enable_construct_t<string, true, T0, Ts...>>
  static string construct(T0 &&arg, Ts &&...args);

  // Enable only for base types of 'value'.
  template <typename T, typename = enable_if_t<std::is_base_of<value, decay_t<T>>::value>>
  static value construct(T &&v) {
    return std::forward<T>(v);
  }

  static value construct() {
    return value();
  }
};

}  // namespace detail
}  // namespace json
}  // namespace spotify
