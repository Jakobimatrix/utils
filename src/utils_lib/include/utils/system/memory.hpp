/**
 * @file memory.hpp
 * @brief Provides a cross-platform API to measure the memory usage of the current process.
 *
 * This header file defines a `MemoryUsage` class with a static method `get()`
 * that returns the memory used by the current process in bytes.
 *
 * ### Supported Platforms
 * - **Windows**: Uses `GetProcessMemoryInfo` from the Windows API. LINK against -lpsapi !!!
 * - **Linux**: Parses `/proc/self/status` for `VmRSS` (Resident Set Size).
 * - **macOS**: Uses `task_info` with `MACH_TASK_BASIC_INFO`.
 *
 * ### References
 * - [Windows API - GetProcessMemoryInfo](https://learn.microsoft.com/en-us/windows/win32/api/psapi/nf-psapi-getprocessmemoryinfo)
 * - [Linux proc filesystem](https://man7.org/linux/man-pages/man5/proc.5.html)
 * - [macOS mach_task_basic_info](https://developer.apple.com/documentation/kernel/mach_task_basic_info_t)
 *
 * @note This implementation throws a `std::runtime_error` on unsupported platforms.
 *
 * @version 1.0
 * @date 2024-12-20
 */

#pragma once

#include <cstddef>
#include <cstdint>

namespace util {
/**
 * @brief Enum to specify memory unit format.
 */
enum class MemoryUnit : std::uint8_t {
  B,   ///< Bytes
  KB,  ///< Kilobytes
  MB,  ///< Megabytes
  GB   ///< Gigabytes
};

/**
 * @class MemoryUsage
 * @brief A utility class to retrieve memory usage of the current process.
 */
class MemoryUsage {
 public:
  /**
   * @brief Get the current memory usage of the process in the specified format.
   *
   * @param unit The unit in which memory usage should be returned (B, KB, MB, GB).
   * @return double Memory usage in the specified unit.
   * @throws std::runtime_error If called on an unsupported platform.
   *
   * ### Example Usage:
   * @code
   * double usageInMB = MemoryUsage::get(MemoryUnit::MB);
   * @endcode
   */
  static double get(MemoryUnit unit);

 private:
#if defined(_WIN32) || defined(_WIN64)
  /**
   * @brief Get memory usage on Windows using `GetProcessMemoryInfo`.
   * @return std::size_t Memory usage in bytes.
   * @see https://learn.microsoft.com/en-us/windows/win32/api/psapi/nf-psapi-getprocessmemoryinfo
   */
  static std::size_t getMemoryUsageWindows();
#elif defined(__linux__)
  /**
   * @brief Get memory usage on Linux by reading `/proc/self/status`.
   * @return std::size_t Memory usage in bytes.
   * @see https://man7.org/linux/man-pages/man5/proc.5.html
   */
  static std::size_t getMemoryUsageLinux();
#elif defined(__APPLE__)
  /**
   * @brief Get memory usage on macOS using `task_info` with `MACH_TASK_BASIC_INFO`.
   * @return std::size_t Memory usage in bytes.
   * @see https://developer.apple.com/documentation/kernel/mach_task_basic_info_t
   */
  static std::size_t getMemoryUsageMac();
#endif
};

}  // namespace util
