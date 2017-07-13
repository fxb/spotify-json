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
#include <cstring>
#include <string>

#include <spotify/json/value/value.hpp>

namespace spotify {
namespace json {

struct string : public value {
 public:
  string();
  string(const std::string &str);
  string(const char *c_str);
  string(const char *c_str, std::size_t size);

  template <typename It>
  string(It begin, It end);

  void reserve(std::size_t new_capacity);

  template <typename It>
  string &append(It begin, It end);

  string &append(const char *c_str, std::size_t size);

  void push_back(char c);

  std::size_t capacity() const;
  std::size_t size() const;
  const char *data() const;
  const char *c_str() const;
  std::string str() const;

  operator const char*() const;
  operator std::string() const;

 private:
  bool is_short_string() const;

  void reserve_short_string(std::size_t new_capacity);
  void reserve_string(std::size_t new_capacity);
};

inline string::string() : string("", 0) {}

inline string::string(const std::string &str) : string(str.c_str(), str.size()) {}

inline string::string(const char *c_str) : string(c_str, std::strlen(c_str)) {}

inline string::string(const char *c_str, std::size_t size) : value(detail::value_union::string) {
  if (size < 16) {
    std::memcpy(_.as_short_string.characters, c_str, size);
    if (size < 15) {
      _.as_short_string.characters[size] = 0;
    }
    _.as_short_string.type = static_cast<detail::value_union::type>(15 - size);
  } else {
    const auto capacity_2exp = detail::value_union::capacity_2exp(size + 1);
    const auto capacity = detail::value_union::capacity(capacity_2exp);
    auto ptr = static_cast<char *>(std::malloc(capacity));
    if (json_unlikely(!ptr)) {
      throw std::bad_alloc();
    }
    _.as_string.characters.ptr = ptr;
    std::memcpy(_.as_string.characters.ptr, c_str, size);
    _.as_string.characters.ptr[size] = 0;
    _.as_string.size = size;
    _.as_string.capacity_2exp = capacity_2exp;
    _.as_string.type = detail::value_union::type::string;
  }
}

template <typename It>
inline string::string(It begin, It end) : value(detail::value_union::string) {
  const std::size_t size = std::distance(begin, end);
  if (size < 16) {
    std::copy(begin, end, _.as_short_string.characters);
    if (size < 15) {
      _.as_short_string.characters[size] = 0;
    }
    _.as_short_string.type = static_cast<detail::value_union::type>(15 - size);
  } else {
    const auto capacity_2exp = detail::value_union::capacity_2exp(size + 1);
    const auto capacity = detail::value_union::capacity(capacity_2exp);
    auto ptr = static_cast<char *>(std::malloc(capacity));
    if (json_unlikely(!ptr)) {
      throw std::bad_alloc();
    }
    _.as_string.characters.ptr = ptr;
    std::copy(begin, end, _.as_string.characters.ptr);
    _.as_string.characters.ptr[size] = 0;
    _.as_string.size = size;
    _.as_string.capacity_2exp = capacity_2exp;
    _.as_string.type = detail::value_union::type::string;
  }
}

inline void string::reserve(std::size_t new_capacity) {
  const auto old_capacity = capacity();
  if (json_unlikely(old_capacity < new_capacity)) {
    if (is_short_string()) {
      reserve_short_string(new_capacity);
    } else {
      reserve_string(new_capacity);
    }
  }
}

inline void string::reserve_short_string(std::size_t new_capacity) {
  if (json_likely(new_capacity >= 16)) {
    const auto size = string::size();
    const auto capacity_2exp = detail::value_union::capacity_2exp(new_capacity);
    const auto capacity = detail::value_union::capacity(capacity_2exp);
    auto ptr = static_cast<char *>(std::malloc(capacity));
    if (json_unlikely(!ptr)) {
      throw std::bad_alloc();
    }
    std::memcpy(ptr, _.as_short_string.characters, size);
    _.as_string.characters.ptr = ptr;
    _.as_string.characters.ptr[size] = 0;
    _.as_string.size = size;
    _.as_string.capacity_2exp = capacity_2exp;
    _.as_string.type = detail::value_union::type::string;
  }
}

inline void string::reserve_string(std::size_t new_capacity) {
  const auto size = string::size();
  const auto capacity_2exp = detail::value_union::capacity_2exp(new_capacity);
  const auto capacity = detail::value_union::capacity(capacity_2exp);
  auto ptr = static_cast<char *>(std::realloc(_.as_string.characters.ptr, capacity));
  if (json_unlikely(!ptr)) {
    throw std::bad_alloc();
  }
  _.as_string.characters.ptr = ptr;
  _.as_string.characters.ptr[size] = 0;
  _.as_string.size = size;
  _.as_string.capacity_2exp = capacity_2exp;
}

template <typename It>
inline string &string::append(It begin, It end) {
  const std::size_t old_size = size();
  const std::size_t new_size = old_size + std::distance(begin, end);
  reserve(new_size + 1);
  if (is_short_string()) {
    std::copy(begin, end, _.as_short_string.characters + old_size);
    if (new_size < 15) {
      _.as_short_string.characters[new_size] = 0;
    }
    _.as_short_string.type = static_cast<detail::value_union::type>(15 - new_size);
  } else {
    std::copy(begin, end, _.as_string.characters.ptr + old_size);
    _.as_string.characters.ptr[new_size] = 0;
    _.as_string.size = new_size;
  }
  return *this;
}

inline string &string::append(const char *c_str, std::size_t size) {
  return append(c_str, c_str + size);
}

inline void string::push_back(char c) {
  const std::size_t old_size = size();
  const std::size_t new_size = old_size + 1;
  reserve(new_size + 1);
  if (is_short_string()) {
    _.as_short_string.characters[old_size] = c;
    if (new_size < 15) {
      _.as_short_string.characters[new_size] = 0;
    }
    _.as_short_string.type = static_cast<detail::value_union::type>(15 - new_size);
  } else {
    _.as_string.characters.ptr[old_size] = c;
    _.as_string.characters.ptr[new_size] = 0;
    _.as_string.size = new_size;
  }
}

inline std::size_t string::capacity() const {
  return is_short_string() ? 15 : detail::value_union::capacity(_.as_string.capacity_2exp);
}

inline std::size_t string::size() const {
  return is_short_string() ?
      15 - static_cast<std::size_t>(_.as_short_string.type) :
      _.as_string.size;
}

inline const char *string::data() const {
  return is_short_string() ?
      _.as_short_string.characters :
      _.as_string.characters.ptr;
}

inline const char *string::c_str() const {
  return data();
}

inline std::string string::str() const {
  return std::string(data(), size());
}

inline string::operator const char*() const {
  return c_str();
}

inline string::operator std::string() const {
  return str();
}

inline bool string::is_short_string() const {
  return (_.as_value.type <= detail::value_union::short_string_00);
}

namespace detail {

template <>
struct construct_impl<string> {
  template <typename ...Args>
  static string construct(Args &&...args) {
    return string(std::forward<Args>(args)...);
  }
};

template <typename T0, typename ...Ts, typename>
string construct_impl<value>::construct(T0 &&arg0, Ts &&...args) {
  return construct_impl<string>::construct(std::forward<T0>(arg0), std::forward<Ts>(args)...);
}

}  // namespace detail
}  // namespace json
}  // namespace spotify
