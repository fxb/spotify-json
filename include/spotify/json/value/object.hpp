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
#include <string>
#include <utility>

#include <spotify/json/type.hpp>
#include <spotify/json/value_exception.hpp>
#include <spotify/json/value/value.hpp>

namespace spotify {
namespace json {

template <typename value_type>
struct object final : public value {
 public:
  object();

  bool empty() const;
  std::size_t size() const;

  value_type &operator[](const std::string &key);
  const value_type &operator[](const std::string &key) const;

  void clear();
  void reserve(std::size_t size);
};

template <typename value_type>
object<value_type>::object() : value(detail::value_union::object) {
  _.as_object.slots.ptr = nullptr;
  _.as_object.size = 0;
  _.as_object.capacity_2exp = 0;
}

template <typename value_type>
bool object<value_type>::empty() const {
  return !_.as_object.size;
}

template <typename value_type>
std::size_t object<value_type>::size() const {
  return _.as_object.size;
}

template <typename value_type>
value_type &object<value_type>::operator[](const std::string &key) {

}

template <typename value_type>
const value_type &object<value_type>::operator[](const std::string &key) const {
  return const_cast<object<value_type> &>(*this)[key];
}

template <typename value_type>
void object<value_type>::clear() {
  _.as_object.size = 0;
}

}  // namespace json
}  // namespace spotify
