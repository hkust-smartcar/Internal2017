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
  for (Uint i = 0; i < width * height / 8; ++i) {
    std::string s = std::bitset<8>((&src)[i]).to_string();
    for (uint8_t j = 0; j < 8; ++j) {
      dest->at(i * 8 / width).at((i * 8 % width) + j) = static_cast<Uint>(s.at(j) - '0') == 1;
    }
  }
}

template<size_t width, size_t height>
void MedianFilter(const std::array<std::array<bool, width>, height> &src,
                  std::array<std::array<bool, width>, height> *dest) {
  for (Uint i = 1; i < height - 1; ++i) {
    for (Uint j = 1; j < width - 1; ++j) {
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

template<size_t width, size_t height>
void MedianFilter(std::array<std::array<bool, width>, height> *arr) {
  std::array<std::array<bool, width>, height> tmp{};
  for (Uint i = 1; i < height - 1; ++i) {
    for (Uint j = 1; j < width - 1; ++j) {
      tmp.at(i).at(j) = static_cast<bool>((
          arr->at(i - 1).at(j - 1) +
              arr->at(i - 1).at(j) +
              arr->at(i - 1).at(j + 1) +
              arr->at(i).at(j - 1) +
              arr->at(i).at(j) +
              arr->at(i).at(j + 1) +
              arr->at(i + 1).at(j - 1) +
              arr->at(i + 1).at(j) +
              arr->at(i + 1).at(j + 1))
          / 5);
    }
  }
  *arr = tmp;
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

template<size_t size>
float CalcLinearRegressionSlope(const std::array<int, size> &x, const std::array<int, size> &y) {
  std::array<std::array<int, 2>, 2> lhs_matrix{{{0, 0}, {0, 0}}};
  std::array<int, 2> rhs_matrix{{0, 0}};

  // least squares method approximation
  for (unsigned int i = 0; i < size; ++i) {
    lhs_matrix.at(0).at(0) += (x.at(i) * x.at(i));
    lhs_matrix.at(0).at(1) += x.at(i);
    lhs_matrix.at(1).at(0) += x.at(i);
    lhs_matrix.at(1).at(1) += 1;
    rhs_matrix.at(0) += (x.at(i) * y.at(i));
    rhs_matrix.at(1) += y.at(i);
  }

  // cramer's rule
  float det = (lhs_matrix.at(0).at(0) * lhs_matrix.at(1).at(1)) - (lhs_matrix.at(1).at(0) * lhs_matrix.at(0).at(1));
  float m = ((rhs_matrix.at(0) * lhs_matrix.at(1).at(1)) - (lhs_matrix.at(0).at(1) * rhs_matrix.at(1))) / det;

  return det != 0 ? m : std::numeric_limits<float>::infinity();
}
}  // namespace util
