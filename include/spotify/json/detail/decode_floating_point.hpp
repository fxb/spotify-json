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

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <limits>

#include <double-conversion/double-conversion.h>

#include <spotify/json/decode_context.hpp>
#include <spotify/json/detail/decode_helpers.hpp>

namespace spotify {
namespace json {
namespace detail {

template <typename T>
T decode_floating_point(
    const double_conversion::StringToDoubleConverter &converter,
    const char* buffer,
    int length,
    int* processed_characters_count);

template <>
inline float decode_floating_point(
    const double_conversion::StringToDoubleConverter &converter,
    const char* buffer,
    int length,
    int* processed_characters_count) {
  return converter.StringToFloat(buffer, length, processed_characters_count);
}

template <>
inline double decode_floating_point(
    const double_conversion::StringToDoubleConverter &converter,
    const char* buffer,
    int length,
    int* processed_characters_count) {
  return converter.StringToDouble(buffer, length, processed_characters_count);
}

template <typename T>
inline T decode_floating_point(decode_context &context) {
  using atod_converter = double_conversion::StringToDoubleConverter;
  static const atod_converter converter(
      atod_converter::ALLOW_TRAILING_JUNK,
      std::numeric_limits<T>::quiet_NaN(),
      std::numeric_limits<T>::quiet_NaN(),
      nullptr,
      nullptr);

  int bytes_read = 0;
  const auto result = decode_floating_point<T>(
      converter,
      context.position,
      static_cast<int>(context.end - context.position),
      &bytes_read);
  fail_if(context, std::isnan(result), "Invalid floating point number");
  skip_unchecked_n(context, bytes_read);
  return result;
}

}  // namespace detail
}  // namespace json
}  // namespace spotify
