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
}  // namespace util
