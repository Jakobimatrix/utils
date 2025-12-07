/**
 * @file benchmark.hpp
 * @brief Defines Makros that allow benchmarking things the compiler would throw away.
 *
 * @author Jakob Wandel
 * @date 2025.09.14
 */

#pragma once

#define BENCHMARK_ALWAYS_INLINE __attribute__((always_inline))

template <class Tp>
inline BENCHMARK_ALWAYS_INLINE void DoNotOptimize(Tp const& value) {
  asm volatile("" : : "r,m"(value) : "memory");
}

template <class Tp>
inline BENCHMARK_ALWAYS_INLINE void DoNotOptimize(Tp& value) {
#if defined(__clang__)
  asm volatile("" : "+r,m"(value) : : "memory");
#else
  asm volatile("" : "+m,r"(value) : : "memory");
#endif
}
