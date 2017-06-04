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

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <utility>

#include <spotify/json/type.hpp>
#include <spotify/json/value_exception.hpp>
#include <spotify/json/value/construct.hpp>
#include <spotify/json/value/value.hpp>

namespace spotify {
namespace json {

template <typename value_type = value>
struct array final : public value {
 public:
  array();

  bool empty() const;
  std::size_t size() const;

  value_type &operator[](std::size_t index);
  const value_type &operator[](std::size_t index) const;

  void clear();

  template <typename ...arg_types>
  void push_back(arg_types &&...args);

  void reserve(std::size_t size);

  using iterator = value_type *;
  using const_iterator = const value_type *;

  iterator begin();
  iterator end();
  const_iterator begin() const;
  const_iterator end() const;
};

template <typename value_type>
array<value_type>::array() : value(detail::value_union::array) {
  _.as_array.elements.ptr = nullptr;
  _.as_array.size = 0;
  _.as_array.capacity_2exp = 0;
}

template <typename value_type>
bool array<value_type>::empty() const {
  return !_.as_array.size;
}

template <typename value_type>
std::size_t array<value_type>::size() const {
  return _.as_array.size;
}

template <typename value_type>
value_type &array<value_type>::operator[](std::size_t index) {
  return reinterpret_cast<value_type &>(_.as_array.elements.ptr[index]);
}

template <typename value_type>
const value_type &array<value_type>::operator[](std::size_t index) const {
  return const_cast<array<value_type> &>(*this)[index];
}

template <typename value_type>
void array<value_type>::clear() {
  _.as_array.size = 0;
}

template <typename value_type>
template <typename ...arg_types>
void array<value_type>::push_back(arg_types &&...args) {
  reserve(size() + 1);  // may throw std::bad_alloc
  reinterpret_cast<value_type &>(_.as_array.elements.ptr[size()]) = detail::construct<value_type>(std::forward<arg_types>(args)...);
  _.as_array.size++;
}

template <typename value_type>
void array<value_type>::reserve(std::size_t size) {
  const auto old_capacity = detail::value_union::capacity(_.as_array.capacity_2exp);
  if (json_unlikely(old_capacity < size)) {
    const auto new_capacity_2exp = std::max(_.as_array.capacity_2exp + 1, 2);
    const auto new_capacity = detail::value_union::capacity(new_capacity_2exp);
    const auto new_elements = new detail::value_union[new_capacity];  // may throw std::bad_alloc
    const auto old_elements = _.as_array.elements.ptr;

    // We know for sure that the move constructor for `value_union` will never
    // throw an exception, so we can simply move over all of the elements from
    // the old array to the new array, without worrying about being interrupted.
    for (auto i = size_t(0); i < _.as_array.size; i++) {
      new_elements[i] = std::move(old_elements[i]);
    }

    _.as_array.capacity_2exp = new_capacity_2exp;
    _.as_array.elements.ptr = new_elements;
    delete[] old_elements;
  }
}

template <typename value_type>
typename array<value_type>::iterator array<value_type>::begin() {
  return reinterpret_cast<value_type *>(_.as_array.elements.ptr);
}

template <typename value_type>
typename array<value_type>::iterator array<value_type>::end() {
  return reinterpret_cast<value_type *>(_.as_array.elements.ptr) + size();
}

template <typename value_type>
typename array<value_type>::const_iterator array<value_type>::begin() const {
  return const_cast<array<value_type> &>(*this).begin();
}

template <typename value_type>
typename array<value_type>::const_iterator array<value_type>::end() const {
  return const_cast<array<value_type> &>(*this).end();
}

}  // namespace json
}  // namespace spotify
