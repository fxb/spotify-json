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

#include <spotify/json/detail/escape.hpp>
#include <spotify/json/detail/macros.hpp>
#include <spotify/json/encode_context.hpp>

namespace spotify {
namespace json {
namespace detail {

template <typename string_type>
json_never_inline void encode_string(encode_context &context, const string_type &value) {
  context.append('"');

  // Write the strings in 1024 byte chunks, so that we do not have to reserve
  // a potentially very large buffer for the escaped string. It is possible
  // that the chunking will happen in the middle of a UTF-8 multi-byte
  // character, but that is ok since write_escaped will not escape characters
  // with the high bit set, so the combined escaped string will contain the
  // correct UTF-8 characters in the end.
  auto chunk_begin = value.data();
  const auto string_end = chunk_begin + value.size();

  while (chunk_begin != string_end) {
    const auto chunk_end = std::min(chunk_begin + 1024, string_end);
    detail::write_escaped(context, chunk_begin, chunk_end);
    chunk_begin = chunk_end;
  }

  context.append('"');
}

}  // namespace detail
}  // namespace json
}  // namespace spotify
