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

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <utility>

#include <spotify/json/encoded_value.hpp>
#include <spotify/json/type.hpp>
#include <spotify/json/value/detail.hpp>
#include <spotify/json/value_exception.hpp>

namespace spotify {
namespace json {
namespace detail {

template <typename value_type> struct cast_impl;

}  // namespace detail

struct value {
 public:
  value();

  type type() const;

 protected:
  template <typename value_type> friend struct detail::cast_impl;

  value(detail::value_union::type type) : _(type) {}
  detail::value_union _;
};

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

}  // namespace json
}  // namespace spotify
