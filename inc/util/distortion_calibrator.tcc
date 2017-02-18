/*
 * Copyright (c) 2014-2017 HKUST SmartCar Team
 * Refer to LICENSE for details
 *
 * Author: David Mak (Derppening)
 */

#include "util/distortion_calibrator.h"

namespace util {
template<size_t width, size_t height>
void ApplyDistortionFilter(std::array<std::array<bool, width>, height> &src,
                           std::array<std::array<bool, width>, height> *dest) {
  for (int i = height; i-- > 0;) {
    for (unsigned int j = 0; j < width / 2; ++j) {
      unsigned int left = static_cast<unsigned>(0 - distortion_slope.at(0) * (79 - i));
      dest->at(i).at(31 - j) = src.at(i).at(left > 63 ? 0 : left);
      unsigned int right = static_cast<unsigned>(63 - distortion_slope.at(1) * (79 - i));
      dest->at(i).at(32 + j) = src.at(i).at(right > 63 ? 63 : right);
    }
  }
}
}
