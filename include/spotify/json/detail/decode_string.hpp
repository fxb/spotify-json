/*
 * Copyright (c) 2016 Spotify AB
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

#include <cstdint>

#include <spotify/json/detail/decode_helpers.hpp>
#include <spotify/json/detail/macros.hpp>
#include <spotify/json/detail/skip_chars.hpp>
#include <spotify/json/decode_context.hpp>

namespace spotify {
namespace json {
namespace detail {

std::uint8_t decode_hex_nibble(decode_context &context, const char c);

template <typename string_type>
void encode_utf8_1(string_type &out, unsigned p) {
  const char c0 = (p & 0x7F);
  out.push_back(c0);
}

template <typename string_type>
void encode_utf8_2(string_type &out, unsigned p) {
  const char c0 = 0xC0 | ((p >> 6) & 0x1F);
  const char c1 = 0x80 | ((p >> 0) & 0x3F);
  const char cc[] = { c0, c1 };
  out.append(&cc[0], 2);
}

template <typename string_type>
void encode_utf8_3(string_type &out, unsigned p) {
  const char c0 = 0xE0 | ((p >> 12) & 0x0F);
  const char c1 = 0x80 | ((p >>  6) & 0x3F);
  const char c2 = 0x80 | ((p >>  0) & 0x3F);
  const char cc[] = { c0, c1, c2 };
  out.append(&cc[0], 3);
}

template <typename string_type>
void encode_utf8(decode_context &context, string_type &out, unsigned p) {
  if (json_likely(p <= 0x7F)) {
    encode_utf8_1(out, p);
  } else if (json_likely(p <= 0x07FF)) {
    encode_utf8_2(out, p);
  } else {
    encode_utf8_3(out, p);
  }
}

template <typename string_type>
void decode_unicode_escape_sequence(decode_context &context, string_type &out) {
  detail::require_bytes<4>(context, "\\u must be followed by 4 hex digits");
  const auto a = decode_hex_nibble(context, *(context.position++));
  const auto b = decode_hex_nibble(context, *(context.position++));
  const auto c = decode_hex_nibble(context, *(context.position++));
  const auto d = decode_hex_nibble(context, *(context.position++));
  const auto p = unsigned((a << 12) | (b << 8) | (c << 4) | d);
  encode_utf8(context, out, p);
}

template <typename string_type>
void decode_escape_sequence(decode_context &context, string_type &out) {
  const auto escape_character = detail::next(context, "Unterminated string");
  switch (escape_character) {
    case '"':  out.push_back('"');  break;
    case '/':  out.push_back('/');  break;
    case 'b':  out.push_back('\b'); break;
    case 'f':  out.push_back('\f'); break;
    case 'n':  out.push_back('\n'); break;
    case 'r':  out.push_back('\r'); break;
    case 't':  out.push_back('\t'); break;
    case '\\': out.push_back('\\'); break;
    case 'u': decode_unicode_escape_sequence(context, out); break;
    default: detail::fail(context, "Invalid escape character", -1);
  }
}

template <typename string_type>
json_never_inline string_type decode_escaped_string(decode_context &context, const char *begin) {
  string_type unescaped(begin, context.position - 1);
  decode_escape_sequence(context, unescaped);

  while (json_likely(context.remaining())) {
    const auto begin_simple = context.position;
    detail::skip_any_simple_characters(context);
    unescaped.append(begin_simple, context.position);

    switch (detail::next(context, "Unterminated string")) {
      case '"': return unescaped;
      case '\\': decode_escape_sequence(context, unescaped); break;
      default: json_unreachable();
    }
  }

  detail::fail(context, "Unterminated string");
}

template <typename string_type>
json_force_inline string_type decode_string(decode_context &context) {
  detail::skip_1(context, '"');

  const auto begin_simple = context.position;
  detail::skip_any_simple_characters(context);

  switch (detail::next(context, "Unterminated string")) {
    case '"': return string_type(begin_simple, context.position - 1);
    case '\\': return decode_escaped_string<string_type>(context, begin_simple);
    default: json_unreachable();
  }
}

}  // namespace detail
}  // namespace json
}  // namespace spotify
