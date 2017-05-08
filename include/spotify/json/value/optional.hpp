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

#include <spotify/json/value/value.hpp>
#include <spotify/json/value_exception.hpp>

namespace spotify {
namespace json {

template <typename value_type>
struct optional final : public value {
 public:
  optional();
  optional(value_type &&inner) noexcept;
  optional(const value_type &inner);

  explicit operator bool() const;
  bool has_value() const;

  value_type *operator->();
  const value_type *operator->() const;

  value_type &operator*() &;
  value_type &&operator*() &&;
  const value_type &operator*() const &;
  const value_type &&operator*() const &&;

  value_type &value() &;
  value_type &value_or(value_type &fallback) &;
  value_type &&value() &&;
  value_type &&value_or(value_type &&fallback) &&;
  const value_type &value() const &;
  const value_type &value_or(value_type &fallback) const &;

  void reset();
  void swap(optional<value_type> &other) noexcept;
};

template <typename value_type>
optional<value_type>::optional() : json::value(detail::value_union::value_null) {}

template <typename value_type>
optional<value_type>::optional(value_type &&inner) noexcept : json::value(std::move(inner)) {}

template <typename value_type>
optional<value_type>::optional(const value_type &inner) : json::value(inner) {}

template <typename value_type>
optional<value_type>::operator bool() const {
  return has_value();
}

template <typename value_type>
bool optional<value_type>::has_value() const {
  return _.as_value.type != detail::value_union::value_null;
}

template <typename value_type>
value_type *optional<value_type>::operator->() {
  return &value();
}

template <typename value_type>
const value_type *optional<value_type>::operator->() const {
  return &value();
}

template <typename value_type>
value_type &optional<value_type>::operator*() & {
  return value();
}

template <typename value_type>
value_type &&optional<value_type>::operator*() && {
  return value();
}

template <typename value_type>
const value_type &optional<value_type>::operator*() const & {
  return value();
}

template <typename value_type>
const value_type &&optional<value_type>::operator*() const && {
  return value();
}

template <typename value_type>
value_type &optional<value_type>::value() & {
  if (has_value()) {
    return static_cast<value_type &>(*this);
  } else {
    throw value_exception("optional<T>::value: not engaged");
  }
}

template <typename value_type>
value_type &optional<value_type>::value_or(value_type &fallback) & {
  return has_value() ? value() : fallback;
}

template <typename value_type>
value_type &&optional<value_type>::value() && {
  if (has_value()) {
    return static_cast<value_type &&>(*this);
  } else {
    throw value_exception("optional<T>::value: not engaged");
  }
}

template <typename value_type>
value_type &&optional<value_type>::value_or(value_type &&fallback) && {
  return has_value() ? value() : fallback;
}

template <typename value_type>
const value_type &optional<value_type>::value() const & {
  return const_cast<optional<value_type> &>(*this).value();
}

template <typename value_type>
const value_type &optional<value_type>::value_or(value_type &fallback) const & {
  return const_cast<optional<value_type> &>(*this).value_or(fallback);
}

template <typename value_type>
void optional<value_type>::reset() {
  _ = detail::value_union(detail::value_union::value_null);
}

template <typename value_type>
void optional<value_type>::swap(optional<value_type> &other) noexcept {
  using std::swap;
  swap(*this, other);
}

}  // namespace json
}  // namespace spotify
