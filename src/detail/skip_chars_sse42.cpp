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

#include <spotify/json/detail/skip_chars.hpp>

#if defined(json_arch_x86)

#include <nmmintrin.h>

#include "skip_chars_common.hpp"

namespace spotify {
namespace json {
namespace detail {
  
json_force_inline unsigned long json_ffs(unsigned x) {
#if defined(_MSC_VER)
  unsigned long offset;
  _BitScanForward(&offset, x);
  return offset;
#elif defined(__GNUC__)
   return (__builtin_ffs(x) - 1);
#endif  // defined(_MSC_VER)
}

void skip_any_simple_characters_sse42(decode_context &context) {
  const auto end = context.end;
  auto pos = context.position;

  JSON_STRING_SKIP_N_SIMPLE(1,  2, uint8_t,  if, done_x)
  JSON_STRING_SKIP_N_SIMPLE(2,  4, uint16_t, if, done_2)
  JSON_STRING_SKIP_N_SIMPLE(4,  8, uint32_t, if, done_4)
  JSON_STRING_SKIP_N_SIMPLE(8, 16, uint64_t, if, done_8)

  {
    alignas(16) static const char CHARS[16] = "\"\\";
    const auto chars = _mm_load_si128(reinterpret_cast<const __m128i *>(&CHARS[0]));

    for (; end - pos >= 16; pos += 16) {
      const auto chunk = _mm_load_si128(reinterpret_cast<const __m128i *>(pos));
      constexpr auto flags = _SIDD_UBYTE_OPS | _SIDD_CMP_EQUAL_ANY | _SIDD_POSITIVE_POLARITY | _SIDD_LEAST_SIGNIFICANT;
      const auto index = _mm_cmpistri(chars, chunk, flags);
      if (index != 16) {
        context.position = pos + index;
        return;
      }
    }
  }

          JSON_STRING_SKIP_N_SIMPLE(8, x, uint64_t, while, done_8)
  done_8: JSON_STRING_SKIP_N_SIMPLE(4, x, uint32_t, while, done_4)
  done_4: JSON_STRING_SKIP_N_SIMPLE(2, x, uint16_t, while, done_2)
  done_2: JSON_STRING_SKIP_N_SIMPLE(1, x, uint8_t,  while, done_x)
  done_x: context.position = pos;
}

void skip_any_simple_characters_avx2(decode_context &context) {
  const auto end = context.end;
  auto pos = context.position;

  for (; pos < end && json_unaligned_32(pos); ++pos) {
    if (*pos == '"' || *pos == '\\') {
      context.position = pos;
      return;
    }
  }

  const auto a = _mm256_set1_epi8('"');
  const auto b = _mm256_set1_epi8('\\');

  for (; end - pos >= 32; pos += 32) {
    const auto chunk = _mm256_load_si256(reinterpret_cast<const __m256i *>(pos));

    __m256i ra = _mm256_cmpeq_epi8(chunk, a);
    __m256i rb = _mm256_cmpeq_epi8(chunk, b);
    __m256i r = _mm256_or_si256(a, b);

    const auto mask = _mm256_movemask_epi8(r);
    if (mask == decltype(mask)(0)) {
      continue;
    }

    const auto index = json_ffs(mask);
    if (index != 16) {
      context.position = pos + index;
      return;
    }
  }

  while (pos < end && *pos != '"' && *pos != '\\') {
    ++pos;
  }

  context.position = pos;
}

void skip_any_whitespace_sse42(decode_context &context) {
  const auto end = context.end;
  auto pos = context.position;

  for (; pos < end && json_unaligned_16(pos); ++pos) {
    if (!is_space(*pos)) {
      context.position = pos;
      return;
    }
  }

  alignas(16) static const char CHARS[16] = " \t\n\r";
  const auto chars = _mm_load_si128(reinterpret_cast<const __m128i *>(&CHARS[0]));

  for (; end - pos >= 16; pos += 16) {
    const auto chunk = _mm_load_si128(reinterpret_cast<const __m128i *>(pos));
    constexpr auto flags = _SIDD_UBYTE_OPS | _SIDD_CMP_EQUAL_ANY | _SIDD_NEGATIVE_POLARITY | _SIDD_LEAST_SIGNIFICANT;
    const auto index = _mm_cmpistri(chars, chunk, flags);
    if (index != 16) {
      context.position = pos + index;
      return;
    }
  }

  while (pos < end && is_space(*pos)) {
    ++pos;
  }

  context.position = pos;
}

void skip_any_whitespace_avx2(decode_context &context) {
  const auto end = context.end;
  auto pos = context.position;

  for (; pos < end && json_unaligned_32(pos); ++pos) {
    if (!is_space(*pos)) {
      context.position = pos;
      return;
    }
  }

  const auto a = _mm256_set1_epi8(' ');
  const auto b = _mm256_set1_epi8('\t');
  const auto c = _mm256_set1_epi8('\n');
  const auto d = _mm256_set1_epi8('\r');

  for (; end - pos >= 32; pos += 32) {
    const auto chunk = _mm256_load_si256(reinterpret_cast<const __m256i *>(pos));

    __m256i ra = _mm256_cmpeq_epi8(chunk, a);
    __m256i rb = _mm256_cmpeq_epi8(chunk, b);
    __m256i rc = _mm256_cmpeq_epi8(chunk, c);
    __m256i rd = _mm256_cmpeq_epi8(chunk, d);
    __m256i r = _mm256_or_si256(
                    _mm256_or_si256(ra, rb),
                    _mm256_or_si256(rc, rd));

    const auto mask = _mm256_movemask_epi8(r);
    if (mask == decltype(mask)(-1)) {
      continue;
    }
    
    const auto index = json_ffs(~mask);
    if (index != 16) {
      context.position = pos + index;
      return;
    }
  }

  while (pos < end && is_space(*pos)) {
    ++pos;
  }

  context.position = pos;
}

}  // namespace detail
}  // namespace json
}  // namespace spotify

#endif  // defined(json_arch_x86)
