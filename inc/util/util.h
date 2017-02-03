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
 * Finds a specified element in an integer-typed C-style array. Searches indices @c [start,end].
 *
 * @note @p start and @p end can be flipped to iterate the array backwards.
 * @note Function does not include bounds checking.
 *
 * @tparam T An integer primitive type
 * @param arr Data array
 * @param first Starting index
 * @param last Ending index
 * @param value The value to find
 * @param return_last If @c true, returns last element if value is not found. Otherwise, returns -1.
 * @return Index of first matching element if found. Otherwise dependent on @p return_last.
 */
template<class T, typename = enable_if_t<std::is_integral<T>::value>>
int find_element(T *arr, int first, int last, T value, bool return_last = true) {
  if (last > first) {
    for (; first <= last; ++first) {
      if (arr[first] == value) {
        return first;
      }
    }
  } else if (first > last) {
    for (; first >= last; --first) {
      if (arr[first] == value) {
        return first;
      }
    }
  } else if (first == last) {
    if (arr[first] == value) {
      return first;
    }
    return return_last ? last : -1;
  }
  return return_last ? last : -1;
}

/**
 * Finds a specified element in an integer-typed C++ standard array. Searches indices @c [start,end].
 *
 * @note @p start and @p end can be flipped to iterate the array backwards.
 * @note Function does not include bounds checking.
 *
 * @tparam T An integer primitive type
 * @param arr Data array
 * @param first Starting index
 * @param last Ending index
 * @param value The value to find
 * @param return_last If @c true, returns last element if value is not found. Otherwise, returns -1.
 * @return Index of first matching element if found. Otherwise dependent on @p return_last.
 */
template<class T, typename = enable_if_t<std::is_integral<T>::value>, std::size_t size>
int find_element(const std::array<T, size> &arr, int first, int last, T value, bool return_last = true) {
  if (last > first) {
    for (; first <= last; ++first) {
      if (arr[first] == value) {
        return first;
      }
    }
  } else if (first > last) {
    for (; first >= last; --first) {
      if (arr[first] == value) {
        return first;
      }
    }
  } else if (first == last) {
    if (arr[first] == value) {
      return first;
    }
    return return_last ? last : -1;
  }
  return return_last ? last : -1;
}
}  // namespace util
