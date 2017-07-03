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

#include <spotify/json/detail/encode_value.hpp>

#include <spotify/json/codec/number.hpp>
#include <spotify/json/detail/encode_string.hpp>
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

void encode_simple_value(encode_context &context, const value &v) {
  switch (v.type()) {
    case type::string: {
      const auto &s = value_cast<string &>(v);
      encode_string(context, s);
      break;
    }
    case type::number: {
      const auto &n = value_cast<json::number &>(v);
      if (n.is_decimal()) {
        codec::number_t<double>().encode(context, n.as<double>());
      } else if (n.is_signed()) {
        codec::number_t<signed long long>().encode(context, n.as<signed long long>());
      } else {
        codec::number_t<unsigned long long>().encode(context, n.as<unsigned long long>());
      }
      break;
    }
    case type::boolean: {
      const auto &b = value_cast<boolean &>(v);
      context.append(b ? "true" : "false", b ? 4 : 5);
      break;
    }
    case type::null: {
      context.append("null", 4);
      break;
    }
    default: {
      break;
    }
  }
}

struct state {
  value value;
  std::size_t offset;
};

void encode_value(encode_context &context, const value &v) {
  stack<state, 64> stack;

  using array_type = array<value>;
  using object_type = object<value>;

  stack.push({v, 0});

  while (!stack.empty()) {
    auto &state = stack.peek();
    if (const auto *arr = value_cast<array_type>(&state.value)) {
      const auto size = arr->size();
      if (json_unlikely(size == 0)) {
        context.append("[]", 2);
      } else if (state.offset < size) {
        context.append(state.offset == 0 ? '[' : ',');
        stack.push({(*arr)[state.offset++], 0});
      } else {
        context.append(']');
        stack.pop();
      }
    } else  if (const auto *obj = value_cast<object_type>(&state.value)) {
      const auto size = obj->size();
      if (json_unlikely(size == 0)) {
        context.append("{}", 2);
      } else if (state.offset < size) {
        context.append(state.offset == 0 ? '{' : ',');
        const auto &entry = (*obj)[state.offset++];
        encode_string(context, entry.first);
        context.append(':');
        stack.push({entry.second, 0});
      } else {
        context.append('}');
        stack.pop();
      }
    } else {
      encode_simple_value(context, stack.pop().value);
    }
  }
}

}  // namespace detail
}  // namespace json
}  // namespace spotify
