/*
 * Copyright (c) 2014-2017 HKUST SmartCar Team
 * Refer to LICENSE for details
 */

#include "../../inc/util/util.h"

namespace util {
template<class T, typename = enable_if_t<std::is_integral<T>::value>>
int find_element(T *arr, int first, int last, T value, bool return_last = false) {
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
