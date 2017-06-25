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
#include <type_traits>

#include <spotify/json/encoded_value.hpp>
#include <spotify/json/type.hpp>
#include <spotify/json/value/construct.hpp>
#include <spotify/json/value/detail.hpp>

namespace spotify {
namespace json {
namespace detail {

template <typename value_type> struct cast_impl;

}  // namespace detail

struct boolean;
struct number;
struct string;

struct value {
 public:
  value();

  template <typename arg_type>
  value &operator=(arg_type &&arg);

  type type() const;

 protected:
  template <typename value_type> friend struct detail::cast_impl;

  value(detail::value_union::type type) : _(type) {}
  detail::value_union _;
};

static_assert(sizeof(value) == sizeof(detail::value_union), "size of value should equal size of value_union");

template <typename T>
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

template <class ...T>
using decay_t = typename std::decay<T...>::type;

template <bool B, class T = void>
using enable_if_t = typename std::enable_if<B, T>::type;

template <typename T, typename ...Args>
using enable_construct_t = enable_if_t<0 < sizeof...(Args) && std::is_constructible<T, Args...>::value && !std::is_same<T, decay_t<Args...>>::value>;

template <>
struct construct_impl<value> {
  static boolean construct(bool b);

  template <typename T, typename = enable_construct_t<number, T>>
  static number construct(T &&v);

  template <typename ...Args, typename = enable_construct_t<string, Args...>>
  static string construct(Args &&...args);

  static value construct(const value &v) {
    return v;
  }

  static value construct() {
    return value();
  }
};

}  // namespace detail
}  // namespace json
}  // namespace spotify
