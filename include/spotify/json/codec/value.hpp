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
#include <spotify/json/detail/encode_value.hpp>
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

  json_never_inline void encode(encode_context &context, const object_type &value) const {
    detail::encode_value(context, value);
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
