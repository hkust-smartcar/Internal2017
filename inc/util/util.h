/*
 * Copyright (c) 2014-2017 HKUST SmartCar Team
 * Refer to LICENSE for details
 */

#pragma once

#include <algorithm>
#include <iterator>
#include <type_traits>

namespace util {
// backport enable_if_t to C++11
template<bool B, class T = void>
using enable_if_t = typename std::enable_if<B, T>::type;

template<class T, typename = enable_if_t<std::is_integral<T>::value>>
int find_element(T *arr, int first, int last, T value);
}  // namespace util
