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

#include <spotify/json/value_exception.hpp>
#include <spotify/json/value/array.hpp>
#include <spotify/json/value/boolean.hpp>
#include <spotify/json/value/detail.hpp>
#include <spotify/json/value/number.hpp>
#include <spotify/json/value/object.hpp>
#include <spotify/json/value/optional.hpp>
#include <spotify/json/value/string.hpp>
#include <spotify/json/value/value.hpp>

namespace spotify {
namespace json {
namespace detail {

template <typename target_type>
struct cast_impl {};

template <>
struct cast_impl<string> {
  using target_type = string;

  static target_type *cast(value *source) noexcept {
    const auto is_string = (source->_.as_value.type <= value_union::string);
    return is_string ? static_cast<target_type *>(source) : nullptr;
  }

  static constexpr const char *message() {
    return "value is not a string";
  }
};

template <typename value_type>
struct cast_impl<array<value_type>> {
  using target_type = array<value_type>;

  static target_type *cast(value *source) {
    const auto is_array = (source->_.as_value.type == value_union::array);
    return is_array ? static_cast<target_type *>(source) : nullptr;
  }

  static constexpr const char *message() {
    return "value is not an array";
  }
};

template <typename value_type>
struct cast_impl<object<value_type>> {
  using target_type = object<value_type>;

  static target_type *cast(value *source) noexcept {
    const auto is_object = (source->_.as_value.type == value_union::object);
    return is_object ? static_cast<target_type *>(source) : nullptr;
  }

  static constexpr const char *message() {
    return "value is not an object";
  }
};

template <>
struct cast_impl<number> {
  using target_type = number;

  static target_type *cast(value *source) noexcept {
    switch (source->_.as_value.type) {
      case value_union::sint64: return static_cast<number *>(source);
      case value_union::uint64: return static_cast<number *>(source);
      case value_union::number: return static_cast<number *>(source);
      default: return nullptr;
    }
  }

  static constexpr const char *message() {
    return "value is not a number";
  }
};

template <>
struct cast_impl<boolean> {
  using target_type = boolean;

  static target_type *cast(value *source) noexcept {
    switch (source->_.as_value.type) {
      case value_union::value_false: return static_cast<boolean *>(source);
      case value_union::value_true:  return static_cast<boolean *>(source);
      default: return nullptr;
    }
  }

  static constexpr const char *message() {
    return "value is not a boolean";
  }
};

}  // namespace detail

/*
 * value_cast<type &>(...)
 */

template <typename target_type>
target_type &&value_cast(value &&source) {
  using cast_impl = detail::cast_impl<typename std::remove_reference<target_type>::type>;
  if (auto ptr = cast_impl::cast(&source)) {
    return std::move(*ptr);
  } else {
    throw value_exception(cast_impl::message());
  }
}

template <typename target_type>
target_type &value_cast(value &source) {
  using cast_impl = detail::cast_impl<typename std::remove_reference<target_type>::type>;
  if (auto ptr = cast_impl::cast(&source)) {
    return *ptr;
  } else {
    throw value_exception(cast_impl::message());
  }
}

template <typename target_type>
const target_type &value_cast(const value &source) {
  return value_cast<target_type &>(const_cast<value &>(source));
}

/*
 * value_cast<type *>(...)
 */

template <typename target_type>
target_type *value_cast(value *source) noexcept {
  using cast_impl = detail::cast_impl<typename std::remove_reference<target_type>::type>;
  return cast_impl::cast(source);
}

template <typename target_type>
const target_type *value_cast(const value *source) noexcept {
  return value_cast<target_type *>(const_cast<value *>(source));
}

}  // namespace json
}  // namespace spotify
