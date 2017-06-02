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

  std::size_t size() const;
  const char *data() const;
  const char *c_str() const;

  operator const char*() const;
  operator std::string() const;

 private:
  bool is_short_string() const;
};

inline string::string() : string("", 0) {}

inline string::string(const std::string &str) : string(str.c_str(), str.size()) {}

inline string::string(const char *c_str) : string(c_str, std::strlen(c_str)) {}

inline string::string(const char *c_str, std::size_t size) : value(detail::value_union::string) {
  if (size < 16) {
    std::memcpy(_.as_short_string.characters, c_str, size);
    _.as_short_string.characters[size] = 0;
    _.as_short_string.type = static_cast<detail::value_union::type>(15 - size);
  } else {
    _.as_string.characters.ptr = new char[size + 1];
    std::memcpy(_.as_string.characters.ptr, c_str, size);
    _.as_string.characters.ptr[size] = 0;
    _.as_string.size = size;
  }
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

inline string::operator const char*() const {
  return c_str();
}

inline string::operator std::string() const {
  return std::string(data(), size());
}

inline bool string::is_short_string() const {
  return (_.as_value.type <= detail::value_union::short_string_00);
}

namespace detail {

template <>
struct construct_impl<string> {
  template <typename ...arg_types>
  static string construct(arg_types &&...args) {
    return string(std::forward<arg_types>(args)...);
  }
};

template <typename ...arg_types, typename>
string construct_impl<value>::construct(arg_types &&...args) {
  return construct_impl<string>::construct(std::forward<arg_types>(args)...);
}

}  // namespace detail
}  // namespace json
}  // namespace spotify
