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

#include <spotify/json/value/array.hpp>
#include <spotify/json/value/string.hpp>
#include <spotify/json/value/number.hpp>
#include <spotify/json/value/value_cast.hpp>
#include <spotify/json/value/value_codec.hpp>

#include <spotify/json/codec/object.hpp>
#include <spotify/json/decode.hpp>

BOOST_AUTO_TEST_SUITE(spotify)
BOOST_AUTO_TEST_SUITE(json)

inline std::ostream &operator <<(std::ostream &stream, const value &v) {
  switch (v.type()) {
    case type::string:
      stream << "string: " <<  (const char *)value_cast<string>(v);
      break;
    case type::object: {
      stream << "object: " << std::endl;
      auto obj = value_cast<object<value>>(v);
      for (auto e : obj) {
        std::cout << "\t" << e.first << ", " << e.second << std::endl;
      }
      break;
    }
    case type::array: {
      stream << "array: " << std::endl;
      auto arr = value_cast<array<value>>(v);
      for (auto e : arr) {
        std::cout << "\t" << e << std::endl;
      }
      break;
    }
    case type::number: {
      stream << "number: ";
      auto n = value_cast<number>(v);
      if (n.is_decimal()) {
        std::cout << n.as<double>() << " (decimal)";
      } else if (n.is_signed()) {
        std::cout << n.as<signed long long>() << " (signed)";
      } else {
        std::cout << n.as<unsigned long long>() << " (unsigned)";
      }
      break;
    }
    case type::boolean: stream << "boolean: " << (bool)value_cast<boolean>(v); break;
    case type::null:    stream << "null"; break;
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

BOOST_AUTO_TEST_CASE(json_value_xxx) {
  codec::value_t codec;

  foo_t foo;

  auto x = decode(codec, R"({
    "foo": 1000,
    "bar": -2000,
    "baz": 3.1415962 
  })");
  std::cout << x << std::endl;

  auto y = decode(codec, R"([1, 2, 3])");
  std::cout << y << std::endl;

  auto z = decode(make_codec(), R"({
    "type": "event",
    "data": {
      "x": 5235,
      "y": 817
    }
  })");
  std::cout << z.type << std::endl;
  std::cout << z.data << std::endl;

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
