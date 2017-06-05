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

#include <spotify/json/detail/decode_string.hpp>

#include <spotify/json/detail/decode_helpers.hpp>

namespace spotify {
namespace json {
namespace detail {

std::uint8_t decode_hex_nibble(decode_context &context, const char c) {
  if (c >= '0' && c <= '9') { return c - '0'; }
  if (c >= 'a' && c <= 'f') { return c - 'a' + 0xA; }
  if (c >= 'A' && c <= 'F') { return c - 'A' + 0xA; }
  detail::fail(context, "\\u must be followed by 4 hex digits");
}

}  // namespace detail
}  // namespace json
}  // namespace spotify
