/*
 * Copyright (c) 2014-2017 HKUST SmartCar Team
 * Refer to LICENSE for details
 *
 * Author: David Mak (Derppening)
 */

#pragma once

namespace util {
/**
 * Calibrates and calculates the perspective distortion of the smartcar.
 *
 * @return Value representing the distortion
 */
float DistortionCalibrator();
}  // namespace util