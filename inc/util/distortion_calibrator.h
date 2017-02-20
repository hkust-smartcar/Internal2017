/*
 * Copyright (c) 2014-2017 HKUST SmartCar Team
 * Refer to LICENSE for details
 *
 * Author: David Mak (Derppening)
 */

#pragma once

#include <array>

namespace util {
/**
 * The slope adjustment constants for the distortion filter.
 */
extern std::array<float, 2> distortion_slope;

/**
 * Calibrates and calculates the perspective distortion of the smartcar.
 *
 * @return Value representing the distortion
 */
bool DistortionCalibrator();

/**
 * Applies the distortion filter to an array.
 *
 * @tparam width Width of the array (size of the interior array)
 * @tparam height Height of the array (size of the exterior array)
 * @param src Source bit (C++11-style) array
 * @param dest Destination bit (C++11-style) array
 */
template<size_t width, size_t height>
void ApplyDistortionFilter(const std::array<std::array<bool, width>, height> &src,
                           std::array<std::array<bool, width>, height> *dest);

/**
 * Applies the distortion filter to an array.
 *
 * @tparam width Width of the array (size of the interior array)
 * @tparam height Height of the array (size of the exterior array)
 * @param arr Bit array (C++11-style) to apply filter to
 */
template<size_t width, size_t height>
void ApplyDistortionFilter(std::array<std::array<bool, width>, height> *arr);

}  // namespace util

#include "util/distortion_calibrator.tcc"
