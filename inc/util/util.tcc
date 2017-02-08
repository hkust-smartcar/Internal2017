/*
 * Copyright (c) 2014-2017 HKUST SmartCar Team
 * Refer to LICENSE for details
 *
 * Author: David Mak (Derppening)
 */

#include "util/util.h"

namespace util {
template<size_t size>
void ByteTo1DBitArray(const Byte &src, std::array<bool, size> *dest) {
  for (Uint i = 0; i < size / 8; ++i) {
    std::string s = std::bitset<8>(static_cast<Uint>((&src)[i])).to_string();
    for (uint8_t j = 0; j < 8; ++j) {
      dest->at(i * 8 + j) = static_cast<Uint>(s.at(j) - '0') == 1;
    }
  }
}

template<size_t width, size_t height>
void ByteTo2DBitArray(const Byte &src, std::array<std::array<bool, width>, height> *dest) {
  for (Uint i = 0; i < width * height; ++i) {
    std::string s = std::bitset<8>((&src)[i]).to_string();
    for (uint8_t j = 0; j < 8; ++j) {
      dest->at(i * 8 / width).at((i * 8 % width) + j) = static_cast<Uint>(s.at(j) - '0') == 1;
    }
  }
}

template<size_t width, size_t height>
void MedianFilter(const std::array<std::array<bool, width>, height> &src,
                  std::array<std::array<bool, width>, height> *dest) {
  for (Uint i = 0; i < height; ++i) {
    for (Uint j = 0; j < width; ++j) {
      if (i == 0) {
        if (j == 0) {
          dest->at(i).at(j) = static_cast<bool>((
              src.at(i).at(j) +
                  src.at(i).at(j + 1) +
                  src.at(i + 1).at(j) +
                  src.at(i + 1).at(j + 1))
              / 3);
        } else if (j == width - 1) {
          dest->at(i).at(j) = static_cast<bool>((
              src.at(i).at(j - 1) +
                  src.at(i).at(j) +
                  src.at(i + 1).at(j - 1) +
                  src.at(i + 1).at(j))
              / 3);
        } else {
          dest->at(i).at(j) = static_cast<bool>((
              src.at(i).at(j - 1) +
                  src.at(i).at(j) +
                  src.at(i).at(j + 1) +
                  src.at(i + 1).at(j - 1) +
                  src.at(i + 1).at(j) +
                  src.at(i + 1).at(j + 1))
              / 4);
        }
      } else if (i == height - 1) {
        if (j == 0) {
          dest->at(i).at(j) = static_cast<bool>((
              src.at(i - 1).at(j) +
                  src.at(i - 1).at(j + 1) +
                  src.at(i).at(j) +
                  src.at(i).at(j + 1))
              / 3);
        } else if (j == width - 1) {
          dest->at(i).at(j) = static_cast<bool>((
              src.at(i - 1).at(j - 1) +
                  src.at(i - 1).at(j) +
                  src.at(i).at(j - 1) +
                  src.at(i).at(j))
              / 3);
        } else {
          dest->at(i).at(j) = static_cast<bool>((
              src.at(i - 1).at(j - 1) +
                  src.at(i - 1).at(j) +
                  src.at(i - 1).at(j + 1) +
                  src.at(i).at(j - 1) +
                  src.at(i).at(j) +
                  src.at(i).at(j + 1))
              / 4);
        }
      } else {
        if (j == 0) {
          dest->at(i).at(j) = static_cast<bool>((
              src.at(i - 1).at(j) +
                  src.at(i - 1).at(j + 1) +
                  src.at(i).at(j) +
                  src.at(i).at(j + 1) +
                  src.at(i + 1).at(j) +
                  src.at(i + 1).at(j + 1))
              / 4);
        } else if (j == width - 1) {
          dest->at(i).at(j) = static_cast<bool>((
              src.at(i - 1).at(j - 1) +
                  src.at(i - 1).at(j) +
                  src.at(i).at(j - 1) +
                  src.at(i).at(j) +
                  src.at(i + 1).at(j - 1) +
                  src.at(i + 1).at(j))
              / 4);
        } else {
          dest->at(i).at(j) = static_cast<bool>((
              src.at(i - 1).at(j - 1) +
                  src.at(i - 1).at(j) +
                  src.at(i - 1).at(j + 1) +
                  src.at(i).at(j - 1) +
                  src.at(i).at(j) +
                  src.at(i).at(j + 1) +
                  src.at(i + 1).at(j - 1) +
                  src.at(i + 1).at(j) +
                  src.at(i + 1).at(j + 1))
              / 5);
        }
      }
    }
  }
}

template<class T, typename = enable_if_t <std::is_integral<T>::value>>
int FindElement(const T &arr, int first, int last, T value, bool return_last) {
  if (last > first) {
    for (; first <= last; ++first) {
      if ((&arr)[first] == value) {
        return first;
      }
    }
  } else if (first > last) {
    for (; first >= last; --first) {
      if ((&arr)[first] == value) {
        return first;
      }
    }
  } else if (first == last) {
    if ((&arr)[first] == value) {
      return first;
    }
    return return_last ? last : -1;
  }
  return return_last ? last : -1;
}

template<class T, typename = enable_if_t <std::is_integral<T>::value>, std::size_t size>
int FindElement(const std::array<T, size> &arr, int first, int last, T value, bool return_last) {
  if (last > first) {
    for (; first <= last; ++first) {
      if (arr.at(first) == value) {
        return first;
      }
    }
  } else if (first > last) {
    for (; first >= last; --first) {
      if (arr.at(first) == value) {
        return first;
      }
    }
  } else if (first == last) {
    if (arr.at(first) == value) {
      return first;
    }
    return return_last ? last : -1;
  }
  return return_last ? last : -1;
}
}  // namespace util
