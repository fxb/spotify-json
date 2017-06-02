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

#include <type_traits>
#include <utility>

namespace spotify {
namespace json {
namespace detail {

template <typename value_type>
struct construct_impl {};

template <typename value_type, typename ...arg_types>
value_type construct(arg_types &&...args) {
  using construct_impl = detail::construct_impl<typename std::remove_reference<value_type>::type>;
  return construct_impl::construct(std::forward<arg_types>(args)...);
}

template <typename value_type>
void destruct(value_type *ptr) {
  ptr->~value_type();
}

}  // namespace detail
}  // namespace json
}  // namespace spotify
