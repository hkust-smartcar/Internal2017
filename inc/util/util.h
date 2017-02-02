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

/**
 * Finds a specified element in an integer-typed array. Searches indices [start, end].
 *
 * @note start and end can be flipped to iterate the array backwards.
 * @note Function does not include bounds checking.
 *
 * @tparam T An integer primitive type
 * @param arr Data array
 * @param first Starting index
 * @param last Ending index
 * @param value The value to find
 * @param return_last If true, returns last element if value is not found. Otherwise, returns -1.
 * @return Index of first matching element if found. Otherwise dependent on return_last.
 */
template<class T, typename = enable_if_t<std::is_integral<T>::value>>
int find_element(T *, int, int, T, bool);
}  // namespace util
