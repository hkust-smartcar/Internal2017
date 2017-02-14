/*
 * Copyright (c) 2014-2017 HKUST SmartCar Team
 * Refer to LICENSE for details
 *
 * Author: David Mak (Derppening)
 */

#include "util/util.h"

#include <string>

using std::bitset;
using std::string;

namespace util {
void CopyByteArray(const Byte &src, Byte *dest, const size_t size) {
  for (Uint i = 0; i < size; ++i) {
    dest[i] = (&src)[i];
  }
}

void ByteTo1DBitArray(const Byte &byte_arr, bool *bit_arr, const size_t size) {
  for (size_t i = 0; i < size; ++i) {
    string s = bitset<8>(static_cast<Uint>((&byte_arr)[i])).to_string();
    for (uint8_t j = 0; j < 8; ++j) {
      bit_arr[i * 8 + j] = static_cast<Uint>(s.at(j) - '0') == 1;
    }
  }
}

float CalcLinearRegressionSlope(const int *x, const int *y, size_t size) {
  int lhs_matrix[2][2] = {{0, 0}, {0, 0}};
  int rhs_matrix[2] = {0, 0};

  // least squares method approximation
  for (unsigned int i = 0; i < size; ++i) {
    lhs_matrix[0][0] += (x[i] * x[i]);
    lhs_matrix[0][1] += x[i];
    lhs_matrix[1][0] += x[i];
    lhs_matrix[1][1] += 1;
    rhs_matrix[0] += (x[i] * y[i]);
    rhs_matrix[1] += y[i];
  }

  // cramer's rule
  float det = lhs_matrix[0][0] * lhs_matrix[1][1] - lhs_matrix[1][0] * lhs_matrix[0][1];
  float m = (rhs_matrix[0] * lhs_matrix[1][1] - lhs_matrix[0][1] * rhs_matrix[1]) / det;

  return det != 0 ? m : std::numeric_limits<float>::infinity();
}
}  // namespace util
