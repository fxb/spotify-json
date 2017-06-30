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

#include <iostream>
#include <vector>

#include <boost/test/unit_test.hpp>

#include <spotify/json/codec/object.hpp>
#include <spotify/json/codec/value.hpp>
#include <spotify/json/encode.hpp>
#include <spotify/json/decode.hpp>
#include <spotify/json/value/array.hpp>
#include <spotify/json/value/string.hpp>
#include <spotify/json/value/number.hpp>
#include <spotify/json/value/value_cast.hpp>

BOOST_AUTO_TEST_SUITE(spotify)
BOOST_AUTO_TEST_SUITE(json)

inline std::ostream &operator <<(std::ostream &stream, const type &t) {
  switch (t) {
    case type::string:  return stream << "string";
    case type::object:  return stream << "object";
    case type::array:   return stream << "array";
    case type::number:  return stream << "number";
    case type::boolean: return stream << "boolean";
    case type::null:    return stream << "null";
  }
  return stream;
}

inline std::ostream &operator <<(std::ostream &stream, const value &v) {
  stream << v.type() << ": ";
  switch (v.type()) {
    case type::string:
      return stream << value_cast<string>(v).str();
    case type::object: {
      stream << "\n";
      auto obj = value_cast<object<value>>(v);
      for (auto e : obj) {
        std::cout << "\t" << e.first << ", " << e.second << '\n';
      }
      break;
    }
    case type::array: {
      stream << "\n";
      auto arr = value_cast<array<value>>(v);
      for (auto e : arr) {
        stream << "\t" << e << '\n';
      }
      break;
    }
    case type::number: {
      auto n = value_cast<number>(v);
      if (n.is_decimal()) {
        stream << n.as<double>() << " (decimal)";
      } else if (n.is_signed()) {
        stream << n.as<signed long long>() << " (signed)";
      } else {
        stream << n.as<unsigned long long>() << " (unsigned)";
      }
      break;
    }
    case type::boolean: return stream << static_cast<bool>(value_cast<boolean>(v));
    case type::null: break;
  }
  return stream;
}

struct foo_t {
  std::string type;
  json::value data;
};

codec::object_t<foo_t> make_codec() {
  codec::object_t<foo_t> codec;
  codec.required("type", &foo_t::type);
  codec.optional("data", &foo_t::data);
  return codec;
}

BOOST_AUTO_TEST_CASE(json_value_construct_from_bool) {
  value v{ true };
  BOOST_REQUIRE_EQUAL(v.type(), type::boolean);
  BOOST_CHECK_EQUAL(static_cast<bool>(value_cast<boolean>(v)), true);
}

BOOST_AUTO_TEST_CASE(json_value_construct_from_arithmetic) {
  value v_c{ 'A' };
  value v_sc{ static_cast<signed char>(42) };
  value v_uc{ static_cast<unsigned char>(42) };
  value v_s{ static_cast<signed short>(42) };
  value v_us{ static_cast<unsigned short>(42) };
  value v_i{ 42 };
  value v_l{ 42l };
  value v_ll{ 42ll };
  value v_ui{ 42u };
  value v_ul{ 42ul };
  value v_ull{ 42ull };
  BOOST_REQUIRE_EQUAL(v_c.type(), type::number);
  BOOST_REQUIRE_EQUAL(v_sc.type(), type::number);
  BOOST_REQUIRE_EQUAL(v_uc.type(), type::number);
  BOOST_REQUIRE_EQUAL(v_s.type(), type::number);
  BOOST_REQUIRE_EQUAL(v_us.type(), type::number);
  BOOST_REQUIRE_EQUAL(v_i.type(), type::number);
  BOOST_REQUIRE_EQUAL(v_l.type(), type::number);
  BOOST_REQUIRE_EQUAL(v_ll.type(), type::number);
  BOOST_REQUIRE_EQUAL(v_ui.type(), type::number);
  BOOST_REQUIRE_EQUAL(v_ul.type(), type::number);
  BOOST_REQUIRE_EQUAL(v_ull.type(), type::number);
  BOOST_CHECK_EQUAL(static_cast<int>(value_cast<number>(v_c)), 65);
  BOOST_CHECK_EQUAL(static_cast<int>(value_cast<number>(v_sc)), 42);
  BOOST_CHECK_EQUAL(static_cast<int>(value_cast<number>(v_uc)), 42);
  BOOST_CHECK_EQUAL(static_cast<int>(value_cast<number>(v_s)), 42);
  BOOST_CHECK_EQUAL(static_cast<int>(value_cast<number>(v_us)), 42);
  BOOST_CHECK_EQUAL(static_cast<int>(value_cast<number>(v_s)), 42);
  BOOST_CHECK_EQUAL(static_cast<int>(value_cast<number>(v_i)), 42);
  BOOST_CHECK_EQUAL(static_cast<int>(value_cast<number>(v_l)), 42);
  BOOST_CHECK_EQUAL(static_cast<int>(value_cast<number>(v_ll)), 42);
  BOOST_CHECK_EQUAL(static_cast<int>(value_cast<number>(v_ui)), 42);
  BOOST_CHECK_EQUAL(static_cast<int>(value_cast<number>(v_ul)), 42);
  BOOST_CHECK_EQUAL(static_cast<int>(value_cast<number>(v_ull)), 42);
}

BOOST_AUTO_TEST_CASE(json_value_construct_from_floating_point) {
  value v_float{ 1.5f };
  value v_double{ 1.5 };
  BOOST_REQUIRE_EQUAL(v_float.type(), type::number);
  BOOST_REQUIRE_EQUAL(v_double.type(), type::number);
  BOOST_CHECK_EQUAL(static_cast<float>(value_cast<number>(v_float)), 1.5f);
  BOOST_CHECK_EQUAL(static_cast<double>(value_cast<number>(v_double)), 1.5);
}

BOOST_AUTO_TEST_CASE(json_value_construct_from_string) {
  value v{ std::string{ "foo" } };
  BOOST_REQUIRE_EQUAL(v.type(), type::string);
  BOOST_CHECK_EQUAL(value_cast<string>(v).str(), "foo");
}

BOOST_AUTO_TEST_CASE(json_value_construct_from_const_char) {
  value v{ "foo" };
  BOOST_REQUIRE_EQUAL(v.type(), type::string);
  BOOST_CHECK_EQUAL(value_cast<string>(v).str(), "foo");
}

BOOST_AUTO_TEST_CASE(json_value_construct_from_const_char_and_size) {
  value v{ "foobar", 3 };
  BOOST_REQUIRE_EQUAL(v.type(), type::string);
  BOOST_CHECK_EQUAL(value_cast<string>(v).str(), "foo");
}

BOOST_AUTO_TEST_CASE(json_value_xxx) {
  number n = 1;
  string s = "foo";

  codec::value_t codec;

  auto x = decode(codec, R"({
    "numbers": [1, 2, 3, [4, 5, 6]],
    "strings": {
      "foo": "bar",
      "bar": "baz"
    },
    "booleans": [true, false]
  })");
  std::cout << x << std::endl;

  foo_t foo;

  /*auto x = decode(codec, R"({
    "foo": 1000,
    "bar": -2000,
    "baz": 3.1415962
  })");
  std::cout << x << std::endl;

  auto y = decode(codec, R"([1, 2, 3])");
  std::cout << y << std::endl;
  //std::cout << "ENCODED" << std::endl;
  //std::cout << encode(codec, y) << std::endl;

  auto z = decode(make_codec(), R"({
    "type": "event",
    "data": {
      "x": 5235,
      "y": 817,
      "z": null
    }
  })");
  std::cout << z.type << std::endl;
  std::cout << z.data << std::endl;*/

  /*object<> o;

  o.emplace("foo", 1);
  o.emplace("bar", "baz");
  o.emplace("d", "1");
  o.emplace("c", "2");
  o.emplace("b", "3");
  o.emplace("a", "4");
  o.emplace("test", "foobar", 3);

  o["fo"] = 3;

  for (auto e : o) {
    std::cout << e.first << ", " << e.second << std::endl;
  }*/

  //std::cout << o["fo"] << std::endl;

  /*value v;
  v = "foobar";
  std::cout << v << std::endl;
  v = 1;
  std::cout << v << std::endl;*/

  /*array<value> a{};

  a.push_back();
  a.push_back(value());
  a.push_back(true);
  a.push_back(false);
  a.push_back((signed char)1);
  a.push_back((signed short)2);
  a.push_back(3);
  a.push_back(4l);
  a.push_back(5ll);
  a.push_back((unsigned char)6u);
  a.push_back((unsigned short)7u);
  a.push_back(8u);
  a.push_back(9ul);
  a.push_back(10ull);
  a.push_back("foobar");
  a.push_back(std::string{"foobar"});
  a.push_back("foobar", 3);
  a.push_back(array<number>());

  for (auto e : a) {
    std::cout << e << std::endl;
  }

  number n(3.1415962);
  double x = n;
  std::cout << x << std::endl;*/

  /*value v;

  v = "foobaraasfklasjflkasjflaksfjaskfjasklfjaslkjaslkfj";
  std::cout << v << std::endl;

  v = 1;
  std::cout << v << std::endl;

  v = value();
  std::cout << v << std::endl;

  const int a = 100;
  v = a;
  std::cout << v << std::endl;*/
}

BOOST_AUTO_TEST_SUITE_END()  // json
BOOST_AUTO_TEST_SUITE_END()  // spotify
