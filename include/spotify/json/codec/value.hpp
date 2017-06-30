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

#include <spotify/json/decode_context.hpp>
#include <spotify/json/default_codec.hpp>
#include <spotify/json/detail/decode_value.hpp>
#include <spotify/json/value/value.hpp>

namespace spotify {
namespace json {
namespace codec {

class value_t final {
 public:
  using object_type = json::value;

  json_never_inline object_type decode(decode_context &context) const {
    return detail::decode_value(context);
  }

  json_never_inline void encode(encode_context &context, const object_type value) const {
    // detail::encode_value(context, value)
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