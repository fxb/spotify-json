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

#include <algorithm>

#include <spotify/json/decode_exception.hpp>
#include <spotify/json/decode_context.hpp>
#include <spotify/json/default_codec.hpp>
#include <spotify/json/detail/decode_helpers.hpp>
#include <spotify/json/detail/decode_string.hpp>
#include <spotify/json/detail/escape.hpp>
#include <spotify/json/detail/macros.hpp>
#include <spotify/json/detail/skip_chars.hpp>
#include <spotify/json/detail/skip_value.hpp>
#include <spotify/json/encode_context.hpp>
#include <spotify/json/codec/number.hpp>
#include <spotify/json/codec/string.hpp>
#include <spotify/json/value/string.hpp>

namespace spotify {
namespace json {
namespace detail {

json_force_inline bool is_digit(const char c) {
  return (c >= '0' && c <= '9');
}

json::number decode_number(decode_context &context) {
  const auto position = context.position;

  bool is_signed = false;
  bool is_decimal = false;

  // Parse negative sign
  if (peek(context) == '-') {
    ++context.position;
    is_signed = true;
  }

  // Parse integer part
  if (peek(context) == '0') {
    ++context.position;
  } else {
    fail_if(context, !is_digit(peek(context)), "Expected digit");
    do { ++context.position; } while (is_digit(peek(context)));
  }

  // Parse fractional part
  if (peek(context) == '.') {
    ++context.position;
    fail_if(context, !is_digit(peek(context)), "Expected digit after decimal point");
    do { ++context.position; } while (is_digit(peek(context)));
    is_decimal = true;
  }

  // Parse exp part
  const char maybe_e = peek(context);
  if (maybe_e == 'e' || maybe_e == 'E') {
    ++context.position;
    const char maybe_plus_minus = peek(context);
    if (maybe_plus_minus == '+' || maybe_plus_minus == '-') {
      ++context.position;
    }

    fail_if(context, !is_digit(peek(context)), "Expected digit after exponent sign");
    do { ++context.position; } while (is_digit(peek(context)));
  }

  context.position = position;

  if (is_decimal) {
    return codec::number_t<double>().decode(context);
  } else if (is_signed) {
    return codec::number_t<signed long long>().decode(context);
  } else {
    return codec::number_t<unsigned long long>().decode(context);
  }
}

}  // namespace detail

namespace codec {

class value_t final {
 public:
  using object_type = value;

  json_never_inline object_type decode(decode_context &context) const {
    detail::require_bytes<1>(context);
    const char c = detail::peek(context);
    if (c == '[') {
      json::array<value> arr;
      detail::decode_comma_separated(context, '[', ']', [&]{
        arr.push_back(decode(context));
      });
      return arr;
    } else if (c == '{') {
      json::object<value> obj;
      detail::decode_object<codec::string_t>(context, [&](std::string &&key) {
        obj.emplace(std::forward<std::string>(key), decode(context));
      });
      return obj;
    } else if (c == 't') {
      detail::skip_true(context);
      return json::boolean(true);
    } else if (c == 'f') {
      detail::skip_false(context);
      return json::boolean(false);
    } else if (c == 'n') {
      detail::skip_null(context);
      return json::value();
    } else if (c == '"') {
      return json::string(codec::string_t().decode(context));
    } else if (c == '-' || detail::is_digit(c)) {
      return detail::decode_number(context);
    } else {
      detail::fail(context, std::string("Encountered unexpected character '") + c + "'");
    }

    return value();
  }

  json_never_inline void encode(encode_context &context, const object_type value) const {

  }
};

inline value_t value() {
  return value_t();
}

}  // namespace codec

template <>
struct default_codec_t<value> {
  static codec::value_t codec() {
    return codec::value_t();
  }
};

}  // namespace json
}  // namespace spotify
