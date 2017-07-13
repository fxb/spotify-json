/*
 * Copyright (c) 2015-2016 Spotify AB
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

#include <type_traits>

#include <spotify/json/decode_context.hpp>
#include <spotify/json/default_codec.hpp>
#include <spotify/json/detail/decode_floating_point.hpp>
#include <spotify/json/detail/decode_integer.hpp>
#include <spotify/json/detail/encode_floating_point.hpp>
#include <spotify/json/detail/encode_helpers.hpp>
#include <spotify/json/detail/encode_integer.hpp>
#include <spotify/json/encode_context.hpp>

namespace spotify {
namespace json {
namespace detail {

template <typename T>
class floating_point_t {
 public:
  using object_type = T;

  object_type decode(decode_context &context) const {
    return decode_floating_point<object_type>(context);
  }

  void encode(encode_context &context, const object_type &value) const {
    encode_floating_point<object_type>(context, value);
  }
};

template <typename T, bool is_integer, bool is_signed>
class integer_t;

template <typename T>
class integer_t<T, true, false> {
 public:
  using object_type = T;

  json_force_inline object_type decode(decode_context &context) const {
    return decode_positive_integer<object_type>(context);
  }

  json_force_inline void encode(encode_context &context, const object_type value) const {
    encode_positive_integer(context, value);
  }
};

template <typename T>
class integer_t<T, true, true> {
 public:
  using object_type = T;

  json_force_inline object_type decode(decode_context &context) const {
    return (peek(context) == '-' ?
        decode_negative_integer<object_type>(context) :
        decode_positive_integer<object_type>(context));
  }

  json_force_inline void encode(encode_context &context, const object_type value) const {
    if (value < 0) {
      encode_negative_integer(context, value);
    } else {
      encode_positive_integer(context, value);
    }
  }
};

template <typename T>
using is_bool = std::is_same<typename std::remove_cv<T>::type, bool>;

}  // namespace detail

namespace codec {

template <typename T>
class number_t final : public detail::integer_t<T,
    std::is_integral<T>::value,
    std::is_signed<T>::value> {
  static_assert(
      std::is_integral<T>::value && !detail::is_bool<T>::value,
      "Trying to use number_t codec for boolean");
};

template <>
class number_t<float> final : public detail::floating_point_t<float> {
};

template <>
class number_t<double> final : public detail::floating_point_t<double> {
};

template <typename T>
number_t<T> number() {
  return number_t<T>();
}

}  // namespace codec

template <typename T>
struct default_codec_t {
  static_assert(
      (std::is_integral<T>::value || std::is_floating_point<T>::value) &&
          !detail::is_bool<T>::value,
      "No default_codec_t specialization for type T");

  static codec::number_t<T> codec() {
    return codec::number_t<T>();
  }
};

}  // namespace json
}  // namespace spotify
