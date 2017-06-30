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

#include <spotify/json/detail/decode_value.hpp>

#include <spotify/json/codec/number.hpp>
#include <spotify/json/codec/string.hpp>
#include <spotify/json/detail/decode_helpers.hpp>
#include <spotify/json/detail/stack.hpp>
#include <spotify/json/value/array.hpp>
#include <spotify/json/value/boolean.hpp>
#include <spotify/json/value/number.hpp>
#include <spotify/json/value/object.hpp>
#include <spotify/json/value/string.hpp>
#include <spotify/json/value/value_cast.hpp>

namespace spotify {
namespace json {
namespace detail {

json_force_inline bool is_digit(const char c) {
  return (c >= '0' && c <= '9');
}

number decode_number(decode_context &context) {
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

value decode_value(decode_context &context) {
  stack<value, 64> stack;
  std::string key;
  value val;

  using array_type = array<value>;
  using object_type = object<value>;

  do {
    const char c = peek(context);
    if (c == '[') {
      skip_1(context, '[');
      skip_any_whitespace(context);

      stack.push(array_type{});

      continue;
    } else if (c == '{') {
      skip_1(context, '{');
      skip_any_whitespace(context);
      key.assign(codec::string_t().decode(context));
      skip_any_whitespace(context);
      skip_1(context, ':');
      skip_any_whitespace(context);

      stack.push(object_type{});

      continue;
    } else if (c == 't') {
      skip_true(context);
      val = boolean{ true };
    } else if (c == 'f') {
      skip_false(context);
      val = boolean{ false };
    } else if (c == 'n') {
      skip_null(context);
      val = value{};
    } else if (c == '"') {
      val = string(codec::string_t().decode(context));
    } else if (c == '-' || is_digit(c)) {
      val = decode_number(context);
    } else {
      fail(context, std::string("Encountered unexpected character '") + c + "'");
    }

    while (!stack.empty()) {
      skip_any_whitespace(context);

      if (auto *arr = value_cast<array_type>(&stack.peek())) {
        arr->push_back(std::move(val));

        if (json_likely(peek(context) != ']')) {
          skip_1(context, ',');
          skip_any_whitespace(context);
          break;
        } else {
          skip_1(context, ']');
          val = stack.pop();
        }
      } else if (auto *obj = value_cast<object_type>(&stack.peek())) {
        obj->emplace(std::move(key), std::move(val));

        if (json_likely(peek(context) != '}')) {
          skip_1(context, ',');
          skip_any_whitespace(context);
          key.assign(codec::string_t().decode(context));
          skip_any_whitespace(context);
          skip_1(context, ':');
          skip_any_whitespace(context);
          break;
        } else {
          skip_1(context, '}');
          val = stack.pop();
        }
      }
    }
  } while (!stack.empty());

  return val;
}

}  // namespace detail
}  // namespace json
}  // namespace spotify
