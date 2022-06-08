#ifndef CPPLIB_ARR2D_HPP
#define CPPLIB_ARR2D_HPP

#include <cstdlib>

namespace arr2d { // stands for `array 2d`

constexpr
size_t get_1d_idx(
  size_t const arrWidth,
  size_t const targetCol,
  size_t const targetRow
) {
  return (targetRow * arrWidth) + targetCol;
}

template<typename ElemT>
constexpr
ElemT max(
  ElemT const *const arr,
  size_t const width,
  size_t const height,
  size_t const startIdx = 0
) {
  ElemT max = arr[startIdx];
  size_t const len = width * height;
  for (size_t i = startIdx + 1; i < len; ++i) {
    if (arr[i] > max) {
      max = arr[i];
    }
  }
  return max;
}

template<typename ElemT>
constexpr
bool cmp(
  ElemT const *const arr1,
  ElemT const *const arr2,
  size_t const width,
  size_t const height,
  size_t const startIdx = 0
) {
  size_t const len = width * height;
  for (size_t i = startIdx; i < len; ++i) {
    if (arr1[i] != arr2[i]) {
      return false;
    }
  }
  return true;
}

template<typename ElemT>
constexpr
bool is_homogenous(
  ElemT const *const arr,
  size_t const width,
  size_t const height,
  size_t const startIdx = 0
) {
  size_t const len = width * height;
  ElemT const &firstElem = arr[startIdx];
  for (size_t i = startIdx + 1; i < len; ++i) {
    if (arr[i] != firstElem) {
      return false;
    }
  }
  return true;
}

} // namespace arr2d

#endif // CPPLIB_ARR2D_HPP
