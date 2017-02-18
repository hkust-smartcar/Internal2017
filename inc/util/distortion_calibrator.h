/*
 * Copyright (c) 2014-2017 HKUST SmartCar Team
 * Refer to LICENSE for details
 *
 * Author: David Mak (Derppening)
 */

#pragma once

#include <array>

namespace util {
extern std::array<float, 2> distortion_slope;

/**
 * Calibrates and calculates the perspective distortion of the smartcar.
 *
 * @return Value representing the distortion
 */
bool DistortionCalibrator();

template<size_t width, size_t height>
void ApplyDistortionFilter(std::array<std::array<bool, width>, height> &src,
                           std::array<std::array<bool, width>, height> *dest);
}  // namespace util

#include "util/distortion_calibrator.tcc"
