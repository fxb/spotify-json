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
#include <cstring>
#include <memory>
#include <string>
#include <tuple>
#include <utility>

#include <spotify/json/type.hpp>
#include <spotify/json/value_exception.hpp>
#include <spotify/json/value/value.hpp>

namespace spotify {
namespace json {

template <typename value_type = value>
struct object final : public value {
 public:
  using entry_type = std::pair<string, value_type>;

  using iterator = entry_type *;
  using const_iterator = const entry_type *;

  object();

  bool empty() const;
  std::size_t size() const;

  value_type &operator[](const std::string &key);
  const value_type &operator[](const std::string &key) const;

  void clear();

  template <typename key_type, typename ...arg_types>
  std::pair<iterator, bool> emplace(key_type &&key, arg_types &&...args);

  void reserve(std::size_t size);

  iterator begin();
  iterator end();
  const_iterator begin() const;
  const_iterator end() const;

 private:
  iterator lower_bound(const std::string &key);

  void move_up(iterator it) {
    for (iterator pos = end(); pos != it; --pos) {
      *pos = std::move(*(pos - 1));
    }
  }

};

template <typename value_type>
object<value_type>::object() : value(detail::value_union::object) {
  _.as_object.entries.ptr = nullptr;
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
  iterator it = lower_bound(key);
  if (it != end() && std::strcmp(key.c_str(), it->first.c_str()) == 0) {
    return it->second;
  } else {
    return emplace(key, value()).first->second;
  }
}

template <typename value_type>
const value_type &object<value_type>::operator[](const std::string &key) const {
  return const_cast<object<value_type> &>(*this)[key];
}

template <typename value_type>
void object<value_type>::clear() {
  _.as_object.size = 0;
}

template <typename value_type>
typename object<value_type>::iterator object<value_type>::lower_bound(const std::string &key) {
  return std::lower_bound(begin(), end(), key,
      [](const entry_type &e, const string &k) { return std::strcmp(e.first.c_str(), k.c_str()) < 0; });
}

template <typename value_type>
template <typename key_type, typename ...arg_types>
std::pair<typename object<value_type>::iterator, bool> object<value_type>::emplace(key_type &&key, arg_types &&...args) {
  iterator it = lower_bound(key);
  if (it != end() && std::strcmp(it->first.c_str(), string(key).c_str()) == 0) {
    return std::make_pair(it, false);
  }

  // reserve may change memory address!
  auto pos = (it - begin());
  reserve(size() + 1);  // may throw std::bad_alloc
  it = begin() + pos;

  if (it != end()) {
    move_up(it);
  }

  *it = entry_type(
      detail::construct<string>(std::forward<key_type>(key)),
      detail::construct<value_type>(std::forward<arg_types>(args)...));

  _.as_object.size++;

  return std::make_pair(it, true);
}

template <typename value_type>
void object<value_type>::reserve(std::size_t size) {
  const auto old_capacity = detail::value_union::capacity(_.as_object.capacity_2exp);
  if (json_unlikely(old_capacity < size)) {
    const auto new_capacity_2exp = std::max(_.as_object.capacity_2exp + 1, 2);
    const auto new_capacity = detail::value_union::capacity(new_capacity_2exp);
    const auto new_entries = new entry_type[new_capacity];  // may throw std::bad_alloc
    const auto old_entries = reinterpret_cast<entry_type *>(_.as_object.entries.ptr);

    // We know for sure that the move constructor for `entry_type` will never
    // throw an exception, so we can simply move over all of the entries from
    // the old object to the new object, without worrying about being interrupted.
    for (auto i = size_t(0); i < _.as_object.size; i++) {
      new_entries[i] = std::move(old_entries[i]);
    }

    _.as_object.capacity_2exp = new_capacity_2exp;
    _.as_object.entries.ptr = reinterpret_cast<detail::value_union::key_value *>(new_entries);
    delete[] old_entries;
  }
}

template <typename value_type>
typename object<value_type>::iterator object<value_type>::begin() {
  return reinterpret_cast<entry_type *>(_.as_object.entries.ptr);
}

template <typename value_type>
typename object<value_type>::iterator object<value_type>::end() {
  return reinterpret_cast<entry_type *>(_.as_object.entries.ptr) + size();
}

template <typename value_type>
typename object<value_type>::const_iterator object<value_type>::begin() const {
  return const_cast<object<value_type> &>(*this).begin();
}

template <typename value_type>
typename object<value_type>::const_iterator object<value_type>::end() const {
  return const_cast<object<value_type> &>(*this).end();
}

}  // namespace json
}  // namespace spotify
