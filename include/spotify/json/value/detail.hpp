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

  enum type : char {
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
    std::uint64_t size:56;
    type type;
  } as_string;

  struct {
    ptr64<key_value> slots;
    std::uint64_t size:48;
    uint8_t capacity_2exp;
    type type;
  } as_object;

  struct {
    ptr64<value_union> elements;
    std::uint64_t size:48;
    uint8_t capacity_2exp;
    type type;
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

  static std::size_t capacity(uint8_t capacity_2exp) {
    return (1 << capacity_2exp) - 1;
  }

 private:
  void move_string(value_union &&other);
  void move_object(value_union &&other);
  void move_array (value_union &&other);
  void duplicate_string();
  void duplicate_object();
  void duplicate_array();
};

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
    case string: delete[] as_string.characters.ptr; break;
    case object: delete[] as_object.slots.ptr; break;
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
  other.as_object.slots.ptr = nullptr;
  other.as_object.size = 0;
  other.as_object.capacity_2exp = 0;
}

inline void value_union::move_array(value_union &&other) {
  other.as_array.elements.ptr = nullptr;
  other.as_array.size = 0;
  other.as_array.capacity_2exp = 0;
}

inline void value_union::duplicate_string() {
  const auto old_characters = as_string.characters.ptr;
  const auto new_characters = new char[as_string.size + 1];
  as_string.characters.ptr = nullptr;
  std::copy(old_characters, &old_characters[as_string.size + 1], new_characters);
  as_string.characters.ptr = new_characters;
}

inline void value_union::duplicate_object() {
  auto old_slots = as_object.slots.ptr;
  auto new_slots = new key_value[capacity(as_object.capacity_2exp)];
  as_object.slots.ptr = nullptr;
  std::copy(old_slots, &old_slots[as_object.size], new_slots);
  as_object.slots.ptr = new_slots;
}

inline void value_union::duplicate_array() {
  const auto old_elements = as_array.elements.ptr;
  const auto new_elements = new value_union[capacity(as_array.capacity_2exp)];
  as_array.elements.ptr = nullptr;
  std::copy(old_elements, &old_elements[as_array.size], new_elements);
  as_array.elements.ptr = new_elements;
}

}  // namespace detail
}  // namespace json
}  // namespace spotify
