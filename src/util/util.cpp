/*
 * Copyright (c) 2014-2017 HKUST SmartCar Team
 * Refer to LICENSE for details
 */

#include "../../inc/util/util.h"

namespace util {
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
 * @return Index of first matching element if found. -1 otherwise.
 */
template<class T, typename = enable_if_t<std::is_integral<T>::value>>
int find_element(T *arr, int first, int last, T value) {
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
    return arr[first] == value ? first : -1;
  }
  return -1;
}
}  // namespace util
