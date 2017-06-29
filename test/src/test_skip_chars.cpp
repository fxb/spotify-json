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

#include <cstdlib>
#include <type_traits>
#include <iostream>

#include <boost/mpl/list.hpp>
#include <boost/test/unit_test.hpp>

#include <spotify/json/detail/skip_chars.hpp>

BOOST_AUTO_TEST_SUITE(spotify)
BOOST_AUTO_TEST_SUITE(json)
BOOST_AUTO_TEST_SUITE(detail)

namespace {

struct cpu_ext_none {};
struct cpu_ext_sse42 {};
struct cpu_ext_avx2 {};

using cpu_ext_list = boost::mpl::list<
    cpu_ext_none,
    cpu_ext_sse42,
    cpu_ext_avx2>;

std::string generate(const std::string &tpl, const std::size_t count) {
  std::string ws;
  ws.reserve(count);
  for (std::size_t i = 0; i < count; i++) {
    ws += tpl[i % tpl.size()];
  }
  return ws;
}

template <void (*function)(decode_context &), typename cpu_ext>
void verify_skip_any(
    const std::string &json,
    const std::size_t prefix = 0,
    const std::size_t suffix = 0) {
  auto context = decode_context(json.data() + prefix, json.data() + json.size());
  *const_cast<bool *>(&context.has_sse42) &= std::is_same<cpu_ext, cpu_ext_sse42>::value;
  *const_cast<bool *>(&context.has_avx2) &= std::is_same<cpu_ext, cpu_ext_avx2>::value;
  const auto original_context = context;
  function(context);
  BOOST_CHECK_EQUAL(
      reinterpret_cast<intptr_t>(context.position),
      reinterpret_cast<intptr_t>(original_context.end - suffix));
  BOOST_CHECK_EQUAL(
      reinterpret_cast<intptr_t>(context.end),
      reinterpret_cast<intptr_t>(original_context.end));
}

template <void (*function)(decode_context &), typename cpu_ext>
void verify_skip_empty_nullptr() {
  auto context = decode_context(nullptr, nullptr);
  std::cout << typeid(cpu_ext).name() << std::endl;
  std::cout << std::is_same<cpu_ext, cpu_ext_sse42>::value << std::endl;
  std::cout << std::is_same<cpu_ext, cpu_ext_avx2>::value << std::endl;
  *const_cast<bool *>(&context.has_sse42) &= std::is_same<cpu_ext, cpu_ext_sse42>::value;
  *const_cast<bool *>(&context.has_avx2) &= std::is_same<cpu_ext, cpu_ext_avx2>::value;
  std::cout << context.has_sse42 << std::endl;
  std::cout << context.has_avx2 << std::endl;
  function(context);
  BOOST_CHECK(context.position == nullptr);
  BOOST_CHECK(context.end == nullptr);
}

}  // namespace

/*
 * skip_any_simple_characters
 */

BOOST_AUTO_TEST_CASE_TEMPLATE(json_skip_any_simple_characters, cpu_ext, cpu_ext_list) {
  for (auto n = 0; n < 1024; n++) {
    const auto ws = generate("abcdefghIJKLMNOP:-,;'^¨´`xyz", n);
    const auto with_prefix = "\\" + ws;
    const auto with_suffix = ws + "\"abcde";
    verify_skip_any<skip_any_simple_characters, cpu_ext>(ws);
    verify_skip_any<skip_any_simple_characters, cpu_ext>(with_prefix, 1);
    verify_skip_any<skip_any_simple_characters, cpu_ext>(with_suffix, 0, 6);
  }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(json_skip_any_simple_characters_with_empty_string,
                              cpu_ext,
                              cpu_ext_list) {
  verify_skip_empty_nullptr<skip_any_simple_characters, cpu_ext>();
}

/*
 * skip_any_whitespace
 */

BOOST_AUTO_TEST_CASE_TEMPLATE(json_skip_any_space, cpu_ext, cpu_ext_list) {
  for (auto n = 0; n < 1024; n++) {
    const auto ws = generate(" ", n);
    const auto with_prefix = "}" + ws;
    const auto with_suffix = ws + "{ ";
    verify_skip_any<skip_any_whitespace, cpu_ext>(ws);
    verify_skip_any<skip_any_whitespace, cpu_ext>(with_prefix, 1);
    verify_skip_any<skip_any_whitespace, cpu_ext>(with_suffix, 0, 2);
  }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(json_skip_any_tabs, cpu_ext, cpu_ext_list) {
  for (auto n = 0; n < 1024; n++) {
    const auto ws = generate("\t", n);
    const auto with_prefix = "}" + ws;
    const auto with_suffix = ws + "{ ";
    verify_skip_any<skip_any_whitespace, cpu_ext>(ws);
    verify_skip_any<skip_any_whitespace, cpu_ext>(with_prefix, 1);
    verify_skip_any<skip_any_whitespace, cpu_ext>(with_suffix, 0, 2);
  }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(json_skip_any_carriage_return, cpu_ext, cpu_ext_list) {
  for (auto n = 0; n < 1024; n++) {
    const auto ws = generate("\r", n);
    const auto with_prefix = "}" + ws;
    const auto with_suffix = ws + "{ ";
    verify_skip_any<skip_any_whitespace, cpu_ext>(ws);
    verify_skip_any<skip_any_whitespace, cpu_ext>(with_prefix, 1);
    verify_skip_any<skip_any_whitespace, cpu_ext>(with_suffix, 0, 2);
  }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(json_skip_any_line_feed, cpu_ext, cpu_ext_list) {
  for (auto n = 0; n < 1024; n++) {
    const auto ws = generate("\n", n);
    const auto with_prefix = "}" + ws;
    const auto with_suffix = ws + "{ ";
    verify_skip_any<skip_any_whitespace, cpu_ext>(ws);
    verify_skip_any<skip_any_whitespace, cpu_ext>(with_prefix, 1);
    verify_skip_any<skip_any_whitespace, cpu_ext>(with_suffix, 0, 2);
  }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(json_skip_any_whitespace, cpu_ext, cpu_ext_list) {
  for (auto n = 0; n < 1024; n++) {
    const auto ws = generate("\n\t\r\n", n);
    const auto with_prefix = "}" + ws;
    const auto with_suffix = ws + "{ ";
    verify_skip_any<skip_any_whitespace, cpu_ext>(ws);
    verify_skip_any<skip_any_whitespace, cpu_ext>(with_prefix, 1);
    verify_skip_any<skip_any_whitespace, cpu_ext>(with_suffix, 0, 2);
  }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(json_skip_any_whitespace_with_empty_string, cpu_ext, cpu_ext_list) {
  verify_skip_empty_nullptr<skip_any_whitespace, cpu_ext>();
}

BOOST_AUTO_TEST_SUITE_END()  // detail
BOOST_AUTO_TEST_SUITE_END()  // json
BOOST_AUTO_TEST_SUITE_END()  // spotify
