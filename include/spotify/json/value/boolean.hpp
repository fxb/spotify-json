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

#include <spotify/json/value/value.hpp>

namespace spotify {
namespace json {

struct boolean : public value {
 public:
  boolean();
  boolean(bool b);

  explicit operator bool() const;
};

inline boolean::boolean() : boolean(false) {}

inline boolean::boolean(bool b) : value(b ?
    detail::value_union::value_true :
    detail::value_union::value_false) {}

boolean::operator bool() const {
  return _.as_value.type == detail::value_union::value_true;
}

namespace detail {

template <>
struct construct_impl<boolean> {
  static boolean construct(bool b) {
    return boolean(b);
  }
};

template <typename T, typename>
boolean construct_impl<value>::construct(T &&v) {
  return construct_impl<boolean>::construct(v);
}

}  // namespace detail
}  // namespace json
}  // namespace spotify
