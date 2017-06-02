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

template <typename arg_type>
inline value &value::operator=(arg_type &&arg) {
  *this = detail::construct<value>(std::forward<arg_type>(arg));
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

template <typename value_type, typename ...arg_types>
using enable_construct_for = typename std::enable_if<std::is_constructible<value_type, arg_types...>::value && 0 < sizeof...(arg_types)>::type;

template <>
struct construct_impl<value> {
  static boolean construct(bool b);

  template <typename arg_type, typename = enable_construct_for<number, arg_type>>
  static number construct(arg_type &&arg);

  template <typename ...arg_types, typename = enable_construct_for<string, arg_types...>>
  static string construct(arg_types &&...args);

  template <typename arg_type, typename = typename std::enable_if<std::is_base_of<value, typename std::remove_reference<arg_type>::type>::value>::type>
  static value construct(arg_type &&v) {
    return value(v);
  }

  static value construct() {
    return value();
  }
};

}  // namespace detail
}  // namespace json
}  // namespace spotify
