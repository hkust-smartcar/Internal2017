/*
 * Copyright (c) 2014-2017 HKUST SmartCar Team
 * Refer to LICENSE for details
 *
 * Author: David Mak (Derppening)
 */

#include "util/distortion_calibrator.h"

namespace util {
template<size_t width, size_t height>
void ApplyDistortionFilter(const std::array<std::array<bool, width>, height> &src,
                           std::array<std::array<bool, width>, height> *dest) {
  for (unsigned int i = height; i-- > 0;) {
    for (unsigned int j = 0; j < width / 2; ++j) {
      unsigned int left = 31 - (static_cast<unsigned>(0 - distortion_slope.at(0) * (79 - i)) * j / 31);
      unsigned int right = 32 + (static_cast<unsigned>(63 - distortion_slope.at(1) * (79 - i)) * j / 31);
      if (left > 63) {
        left = 63;
      } else if (left < 0) {
        left = 0;
      }
      if (right > 63) {
        right = 63;
      } else if (right < 0) {
        right = 0;
      }
      dest->at(i).at(31 - j) = src.at(i).at(left < 0 ? 0 : left);
      dest->at(i).at(32 + j) = src.at(i).at(right > 63 ? 63 : right);
    }
  }
}

template<size_t width, size_t height>
void ApplyDistortionFilter(std::array<std::array<bool, width>, height> *arr) {
  std::array<std::array<bool, width>, height> tmp{};
  for (unsigned int i = height; i-- > 0;) {
    for (unsigned int j = 0; j < width / 2; ++j) {
      unsigned int left = 31 - (static_cast<unsigned>(0 - distortion_slope.at(0) * (79 - i)) * j / 31);
      unsigned int right = 32 + (static_cast<unsigned>(63 - distortion_slope.at(1) * (79 - i)) * j / 31);
      if (left > 63) {
        left = 63;
      } else if (left < 0) {
        left = 0;
      }
      if (right > 63) {
        right = 63;
      } else if (right < 0) {
        right = 0;
      }
      tmp.at(i).at(31 - j) = arr->at(i).at(left < 0 ? 0 : left);
      tmp.at(i).at(32 + j) = arr->at(i).at(right > 63 ? 63 : right);
    }
  }
  *arr = tmp;
}
}
