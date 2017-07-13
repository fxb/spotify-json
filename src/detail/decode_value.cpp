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

#include <spotify/json/detail/decode_helpers.hpp>
#include <spotify/json/detail/decode_floating_point.hpp>
#include <spotify/json/detail/decode_integer.hpp>
#include <spotify/json/detail/decode_string.hpp>
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

  bool is_negative = false;
  bool is_decimal = false;

  // Parse negative sign
  if (peek(context) == '-') {
    ++context.position;
    is_negative = true;
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
    return decode_floating_point<double>(context);
  } else if (is_negative) {
    return decode_negative_integer<signed long long>(context);
  } else {
    return decode_positive_integer<unsigned long long>(context);
  }
}

json_force_inline string decode_object_key(decode_context &context) {
  skip_any_whitespace(context);
  string key = decode_string<string>(context);
  skip_any_whitespace(context);
  skip_1(context, ':');
  skip_any_whitespace(context);
  return key;
}

using array_type = array<value>;
using object_type = object<value>;

value decode_value(decode_context &context) {
  stack<value, 64> stack;
  value current;

  do {
    const char c = peek(context);
    if (c == '[') {
      skip_1(context, '[');
      skip_any_whitespace(context);
      if (json_unlikely(peek(context) == ']')) {
        skip_1(context, ']');
        current = array_type{};
      } else {
        stack.push(array_type{});
        continue;
      }
    } else if (c == '{') {
      skip_1(context, '{');
      if (json_unlikely(peek(context) == '}')) {
        skip_1(context, '}');
        current = object_type{};
      } else {
        auto obj = object_type{};
        obj.emplace(decode_object_key(context), value{});
        stack.push(std::move(obj));
        continue;
      }
    } else if (c == 't') {
      skip_true(context);
      current = boolean{ true };
    } else if (c == 'f') {
      skip_false(context);
      current = boolean{ false };
    } else if (c == 'n') {
      skip_null(context);
      current = value{};
    } else if (c == '"') {
      current = decode_string<string>(context);
    } else if (c == '-' || is_digit(c)) {
      current = decode_number(context);
    } else {
      fail(context, std::string("Encountered unexpected character '") + c + "'");
    }

    while (!stack.empty()) {
      skip_any_whitespace(context);

      auto &container = stack.peek();
      if (auto *arr = value_cast<array_type>(&container)) {
        arr->push_back(std::move(current));

        if (json_likely(peek(context) != ']')) {
          skip_1(context, ',');
          skip_any_whitespace(context);
          break;
        } else {
          skip_1(context, ']');
          current = std::move(container);
          stack.pop();
        }
      } else if (auto *obj = value_cast<object_type>(&container)) {
        (*obj)[obj->size() - 1].second = std::move(current);

        if (json_likely(peek(context) != '}')) {
          skip_1(context, ',');
          obj->emplace(decode_object_key(context), value{});
          break;
        } else {
          skip_1(context, '}');
          current = std::move(container);
          stack.pop();
        }
      }
    }
  } while (!stack.empty());

  return current;
}

}  // namespace detail
}  // namespace json
}  // namespace spotify
