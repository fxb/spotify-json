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
#include <utility>

namespace spotify {
namespace json {
namespace detail {

union value_union {
  template <typename T, std::size_t size = sizeof(T *)> struct ptr64 {};
  template <typename T> struct ptr64<T, 4> { T *ptr; uint32_t _; };
  template <typename T> struct ptr64<T, 8> { T *ptr; };

  enum type : std::uint8_t {
    short_string_15 = 0,
    short_string_14 = 1,
    short_string_13 = 2,
    short_string_12 = 3,
    short_string_11 = 4,
    short_string_10 = 5,
    short_string_09 = 6,
    short_string_08 = 7,
    short_string_07 = 8,
    short_string_06 = 9,
    short_string_05 = 10,
    short_string_04 = 11,
    short_string_03 = 12,
    short_string_02 = 13,
    short_string_01 = 14,
    short_string_00 = 15,
    string = 16,
    object = 17,
    array = 18,
    sint64 = 19,
    uint64 = 20,
    number = 21,
    value_false = 22,
    value_true = 23,
    value_null = 24,
    invalid = 25
  };

  using key_value = std::pair<value_union, value_union>;

  value_union();
  value_union(type type);
  value_union(value_union &&other) noexcept;
  value_union(const value_union &other);
  ~value_union();

  value_union &operator=(value_union &&other);
  value_union &operator=(const value_union &other);

  struct header {
    header() {}
    header(type type) : type(type) {}
    char data[15];
    type type;
  } as_value;

  struct {
    char characters[15];
    type type;
  } as_short_string;

  struct {
    ptr64<char> characters;
    std::uint64_t size : 48;
    std::uint64_t capacity_2exp : 8;
    std::uint64_t type : 8;
  } as_string;

  struct {
    ptr64<key_value> entries;
    std::uint64_t size : 48;
    std::uint64_t capacity_2exp : 8;
    std::uint64_t type : 8;
  } as_object;

  struct {
    ptr64<value_union> elements;
    std::uint64_t size : 48;
    std::uint64_t capacity_2exp : 8;
    std::uint64_t type : 8;
  } as_array;

  struct {
    std::int64_t number;
    char padding[7];
    type type;
  } as_sint64;

  struct {
    std::uint64_t number;
    char padding[7];
    type type;
  } as_uint64;

  struct {
    double number;
    char padding[7];
    type type;
  } as_double;

  static std::uint64_t capacity(std::uint64_t capacity_2exp) {
    return (1 << capacity_2exp) - 1;
  }

  static std::uint64_t capacity_2exp(std::uint64_t capacity) {
    return static_cast<std::uint64_t>(std::log2(capacity) + 1);
  }

 private:
  void move_string(value_union &&other);
  void move_object(value_union &&other);
  void move_array (value_union &&other);
  void duplicate_string();
  void duplicate_object();
  void duplicate_array();
};

static_assert(sizeof(value_union::ptr64<char>) == 8, "value_union::ptr64<char> should be exactly 8 bytes");
static_assert(sizeof(value_union::as_value) == 16, "value_union::as_value should be exactly 16 bytes");
static_assert(sizeof(value_union::as_short_string) == 16, "value_union::as_short_string should be exactly 16 bytes");
static_assert(sizeof(value_union::as_string) == 16, "value_union::as_string should be exactly 16 bytes");
static_assert(sizeof(value_union::as_object) == 16, "value_union::as_object should be exactly 16 bytes");
static_assert(sizeof(value_union::as_array) == 16, "value_union::as_array should be exactly 16 bytes");
static_assert(sizeof(value_union::as_sint64) == 16, "value_union::as_sint64 should be exactly 16 bytes");
static_assert(sizeof(value_union::as_uint64) == 16, "value_union::as_uint64 should be exactly 16 bytes");
static_assert(sizeof(value_union::as_double) == 16, "value_union::as_double should be exactly 16 bytes");
static_assert(sizeof(value_union) == 16, "value_union should be exactly 16 bytes");

inline value_union::value_union() : as_value(invalid) {}

inline value_union::value_union(type type) : as_value(type) {}

inline value_union::value_union(value_union &&other) noexcept : as_value(other.as_value) {
  switch (as_value.type) {
    case string: move_string(std::move(other)); break;
    case object: move_object(std::move(other)); break;
    case array:  move_array (std::move(other)); break;
    default: break;
  }
}

inline value_union::value_union(const value_union &other) : as_value(other.as_value) {
  switch (as_value.type) {
    case string: duplicate_string(); break;
    case object: duplicate_object(); break;
    case array:  duplicate_array (); break;
    default: break;
  }
}

inline value_union::~value_union() {
  switch (as_value.type) {
    case string: std::free(as_string.characters.ptr); break;
    case object: delete[] as_object.entries.ptr; break;
    case array:  delete[] as_array.elements.ptr; break;
    default: break;
  }
}

inline value_union &value_union::operator=(value_union &&other) {
  as_value = other.as_value;
  switch (as_value.type) {
    case string: move_string(std::move(other)); break;
    case object: move_object(std::move(other)); break;
    case array:  move_array (std::move(other)); break;
    default: break;
  }
  return *this;
}

inline value_union &value_union::operator=(const value_union &other) {
  as_value = other.as_value;
  switch (as_value.type) {
    case string: duplicate_string(); break;
    case object: duplicate_object(); break;
    case array:  duplicate_array (); break;
    default: break;
  }
  return *this;
}

inline void value_union::move_string(value_union &&other) {
  other.as_short_string.type = short_string_00;
  other.as_short_string.characters[0] = 0;
}

inline void value_union::move_object(value_union &&other) {
  other.as_object.entries.ptr = nullptr;
  other.as_object.size = 0;
  other.as_object.capacity_2exp = 0;
}

inline void value_union::move_array(value_union &&other) {
  other.as_array.elements.ptr = nullptr;
  other.as_array.size = 0;
  other.as_array.capacity_2exp = 0;
}

template<class T>
inline void value_copy_n(const T *in, std::size_t size, T *out) {
  while (size--) {
    *out++ = *in++;
  }
}

inline void value_union::duplicate_string() {
  const auto old_characters = as_string.characters.ptr;
  const auto new_characters = static_cast<char *>(std::malloc(capacity(as_string.capacity_2exp)));
  if (json_unlikely(!new_characters)) {
    throw std::bad_alloc();
  }
  as_string.characters.ptr = nullptr;
  std::memcpy(new_characters, old_characters, as_string.size + 1);
  as_string.characters.ptr = new_characters;
}

inline void value_union::duplicate_object() {
  auto old_entries = as_object.entries.ptr;
  auto new_entries = new key_value[capacity(as_object.capacity_2exp)];
  as_object.entries.ptr = nullptr;
  value_copy_n(old_entries, as_object.size, new_entries);
  as_object.entries.ptr = new_entries;
}

inline void value_union::duplicate_array() {
  const auto old_elements = as_array.elements.ptr;
  const auto new_elements = new value_union[capacity(as_array.capacity_2exp)];
  as_array.elements.ptr = nullptr;
  value_copy_n(old_elements, as_array.size, new_elements);
  as_array.elements.ptr = new_elements;
}

}  // namespace detail
}  // namespace json
}  // namespace spotify
