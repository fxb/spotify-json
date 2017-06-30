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
#include <spotify/json/detail/stack.hpp>
#include <spotify/json/encode_context.hpp>

#include <spotify/json/codec/number.hpp>
#include <spotify/json/codec/string.hpp>

#include <spotify/json/value/array.hpp>
#include <spotify/json/value/boolean.hpp>
#include <spotify/json/value/number.hpp>
#include <spotify/json/value/object.hpp>
#include <spotify/json/value/string.hpp>
#include <spotify/json/value/value.hpp>

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

/*struct state {
  enum stype {
    none,
    array,
    object
  };

  stype type;
  void *begin;
  void *end;

  template <template<typename> class T>
  typename T<json::value>::const_iterator current() {
    return reinterpret_cast<typename T<json::value>::const_iterator>(begin);
  }

  template <template<typename> class T>
  bool next() {
    auto *it_begin = reinterpret_cast<typename T<json::value>::const_iterator>(begin);
    auto *it_end = reinterpret_cast<typename T<json::value>::const_iterator>(end);
    if (it_begin == it_end) {
      return false;
    }
    begin = (void *)(++it_begin);
    return true;
  }
};*/

class value_t final {
 public:
  using object_type = json::value;

  json_never_inline json::value decode(decode_context &context) const {
    detail::stack<json::value, 64> stack;
    std::string key;
    json::value val;

    using array = json::array<json::value>;
    using object = json::object<json::value>;

    do {
      const char c = detail::peek(context);
      if (c == '[') {
        detail::skip_1(context, '[');
        detail::skip_any_whitespace(context);

        stack.push(array{});

        continue;
      } else if (c == '{') {
        detail::skip_1(context, '{');
        detail::skip_any_whitespace(context);
        key.assign(codec::string_t().decode(context));
        detail::skip_any_whitespace(context);
        detail::skip_1(context, ':');
        detail::skip_any_whitespace(context);

        stack.push(object{});

        continue;
      } else if (c == 't') {
        detail::skip_true(context);
        val = json::boolean{ true };
      } else if (c == 'f') {
        detail::skip_false(context);
        val = json::boolean{ false };
      } else if (c == 'n') {
        detail::skip_null(context);
        val = json::value{};
      } else if (c == '"') {
        val = json::string(codec::string_t().decode(context));
      } else if (c == '-' || detail::is_digit(c)) {
        val = detail::decode_number(context);
      } else {
        detail::fail(context, std::string("Encountered unexpected character '") + c + "'");
      }

      while (!stack.empty()) {
        detail::skip_any_whitespace(context);

        if (auto arr = value_cast<array>(&stack.peek())) {
          arr->push_back(std::move(val));

          if (json_likely(detail::peek(context) != ']')) {
            detail::skip_1(context, ',');
            detail::skip_any_whitespace(context);
            break;
          } else {
            detail::skip_1(context, ']');
            val = stack.pop();
          }
        } else if (auto arr = value_cast<object>(&stack.peek())) {
          arr->emplace(std::move(key), std::move(val));

          if (json_likely(detail::peek(context) != '}')) {
            detail::skip_1(context, ',');
            detail::skip_any_whitespace(context);
            key.assign(codec::string_t().decode(context));
            detail::skip_any_whitespace(context);
            detail::skip_1(context, ':');
            detail::skip_any_whitespace(context);
            break;
          } else {
            detail::skip_1(context, '}');
            val = stack.pop();
          }
        }
      }
    } while (!stack.empty());

    return val;
  }

  json_never_inline void encode(encode_context &context, const json::value value) const {
    /*detail::stack<state, 64> stack;
    auto current = value;

    stack.push({state::stype::none, nullptr, nullptr});

    while(stack.size() > 0) {
      state s = stack.peek();

      if (s.type == state::stype::array) {
        current = *s.current<json::array>();
        if (!s.next<json::array>()) {
          std::cout << "end" << std::endl;
          stack.pop();
        }
      } else if (s.type == state::stype::object) {
        auto e = *s.current<json::object>();
        detail::encode_string(context, e.first);
        current = e.second;
        if (!s.next<json::object>()) {
          stack.pop();
        }
      } else {
        std::cout << "val" << std::endl;
        stack.pop();
      }

      switch (current.type()) {
        case type::array: {
          const json::array<json::value> &v = value_cast<json::array<json::value> &>(current);
          context.append('[');
          stack.push({state::stype::array, (void *)v.begin(), (void *)v.end()});
          break;
        }
        case type::object: {
          const json::object<json::value> &v = value_cast<json::object<json::value> &>(current);
          stack.push({state::stype::object, (void *)v.begin(), (void *)v.end()});
          break;
        }
        case type::string: {
          const json::string &v = value_cast<json::string &>(current);
          detail::encode_string(context, v);
          break;
        }
        case type::number: {
          const json::number &v = value_cast<json::number &>(current);
          if (v.is_decimal()) {
            codec::number_t<double>().encode(context, v.as<double>());
          } else if (v.is_signed()) {
            codec::number_t<signed long long>().encode(context, v.as<signed long long>());
          } else {
            codec::number_t<unsigned long long>().encode(context, v.as<unsigned long long>());
          }
          break;
        }
        case type::boolean: {
          const json::boolean &v = value_cast<json::boolean &>(current);
          context.append(v ? "true" : "false", v ? 4 : 5);
          break;
        }
        case type::null: {
          context.append("null", 4);
          break;
        }
      }
    }*/
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
