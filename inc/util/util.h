/*
 * Copyright (c) 2014-2017 HKUST SmartCar Team
 * Refer to LICENSE for details
 *
 * Author: David Mak (Derppening)
 */

#pragma once

#include <algorithm>
#include <array>
#include <bitset>
#include <iterator>
#include <string>
#include <type_traits>

#include "libbase/misc_types.h"

namespace util {
/**
 * Copies an array to another one
 *
 * @param src Source (C-style) array
 * @param dest Destination (C-style) array
 * @param size Size (i.e. number of elements) of the arrays
 */
void CopyByteArray(const Byte &src, Byte *dest, const size_t size);
/**
 * Converts a byte array to a C-style 1D bit array
 *
 * @param byte_arr Source byte (C-style) array
 * @param bit_arr Destination 1D bit (C-style) array
 * @param size Size (i.e. number of elements) of @c src array
 */
void ByteTo1DBitArray(const Byte &src, bool *dest, const size_t size);
/**
 * Converts a byte array to a C++11-style 1D bit array
 *
 * @tparam size Size of @c src array
 * @param src Source byte (C-style) array
 * @param dest Destination 1D bit (C++11-style) array
 */
template<size_t size>
void ByteTo1DBitArray(const Byte &src, std::array<bool, size> *dest);

/**
 * Converts a byte array to a C++11-style 2D bit array
 *
 * @tparam width Width of the array (size of the interior array)
 * @tparam height Height of the array (size of the exterior array)
 * @param src Source byte (C-style) array
 * @param dest Destination bit (C++11-style) array
 */
template<size_t width, size_t height>
void ByteTo2DBitArray(const Byte &src, std::array<std::array<bool, width>, height> *dest);

/**
 * Applies median filter to a C++11-style 2D bit array
 *
 * @tparam width Width of the array (size of the interior array)
 * @tparam height Height of the array (size of the exterior array)
 * @param src Source bit (C++11-style) array
 * @param dest Destination bit (C++11-style) array
 */
template<size_t width, size_t height>
void MedianFilter(const std::array<std::array<bool, width>, height> &src,
                  std::array<std::array<bool, width>, height> *dest);

/**
 * Backport of @c std::enable_if_t from C++14
 */
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
int FindElement(const T &arr, int first, int last, T value, bool return_last = true);

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
int FindElement(const std::array<T, size> &arr, int first, int last, T value, bool return_last = true);
/**
 * Calculates the slope of the linear regression line from a given set of points.
 *
 * @tparam size Size of the arrays
 * @param x Array of x values
 * @param y Array of y values
 * @return Slope of regression line
 */
template<size_t size>
float CalcLinearRegressionSlope(const std::array<int, size> &x, const std::array<int, size> &y);
/**
 * Calculates the slope of the linear regression line from a given set of points.
 *
 * @param x Array of x values
 * @param y Array of y values
 * @param size Size of the arrays
 * @return Slope of regression line
 */
float CalcLinearRegressionSlope(const int *x, const int *y, size_t size);
}  // namespace util

#include "util/util.tcc"
