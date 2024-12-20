/**
 * @file memory.h
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
#include <string>

#if defined(_WIN32) || defined(_WIN64)  // Windows
#include <windows.h>
#include <psapi.h>

#elif defined(__linux__)  // Linux
#include <fstream>
#include <string>

#elif defined(__APPLE__)  // macOS
#include <mach/mach.h>
#endif

namespace util {
/**
 * @brief Enum to specify memory unit format.
 */
enum class MemoryUnit {
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
  static double get(MemoryUnit unit) {
    std::size_t memoryUsageInBytes = 0;

#if defined(_WIN32) || defined(_WIN64)
    memoryUsageInBytes = getMemoryUsageWindows();
#elif defined(__linux__)
    memoryUsageInBytes = getMemoryUsageLinux();
#elif defined(__APPLE__)
    memoryUsageInBytes = getMemoryUsageMac();
#else
    throw std::runtime_error("Unsupported platform");
#endif

    switch (unit) {
      case MemoryUnit::B:
        return static_cast<double>(memoryUsageInBytes);
      case MemoryUnit::KB:
        return memoryUsageInBytes / 1024.0;
      case MemoryUnit::MB:
        return memoryUsageInBytes / (1024.0 * 1024.0);
      case MemoryUnit::GB:
        return memoryUsageInBytes / (1024.0 * 1024.0 * 1024.0);
      default:
        throw std::invalid_argument("Invalid MemoryUnit");
    }
  }

 private:
#if defined(_WIN32) || defined(_WIN64)
  /**
   * @brief Get memory usage on Windows using `GetProcessMemoryInfo`.
   * @return std::size_t Memory usage in bytes.
   * @see https://learn.microsoft.com/en-us/windows/win32/api/psapi/nf-psapi-getprocessmemoryinfo
   */
static std::size_t getMemoryUsageWindows() {
    PROCESS_MEMORY_COUNTERS memInfo;
    // Use K32GetProcessMemoryInfo for compatibility with modern Windows.
    if (K32GetProcessMemoryInfo(GetCurrentProcess(), &memInfo, sizeof(memInfo))) {
        return memInfo.WorkingSetSize; // Bytes
    }
    return 0;
}
#elif defined(__linux__)
  /**
   * @brief Get memory usage on Linux by reading `/proc/self/status`.
   * @return std::size_t Memory usage in bytes.
   * @see https://man7.org/linux/man-pages/man5/proc.5.html
   */
  static std::size_t getMemoryUsageLinux() {
    std::ifstream proc("/proc/self/status");
    std::string line;

    while (std::getline(proc, line)) {
      if (line.find("VmRSS:") != std::string::npos) {  // Resident Set Size
        std::istringstream iss(line);
        std::string key;
        std::size_t value;
        std::string unit;

        iss >> key >> value >> unit;  // Parse key, value, and unit
        if (unit == "kB") {
          return value * 1024;  // Convert from kB to bytes
        }
      }
    }
    return 0;
  }
#elif defined(__APPLE__)
  /**
   * @brief Get memory usage on macOS using `task_info` with `MACH_TASK_BASIC_INFO`.
   * @return std::size_t Memory usage in bytes.
   * @see https://developer.apple.com/documentation/kernel/mach_task_basic_info_t
   */
  static std::size_t getMemoryUsageMac() {
    mach_task_basic_info info;
    mach_msg_type_number_t count = MACH_TASK_BASIC_INFO_COUNT;
    if (task_info(mach_task_self(), MACH_TASK_BASIC_INFO, (task_info_t)&info, &count) ==
        KERN_SUCCESS) {
      return info.resident_size;  // Bytes
    }
    return 0;
  }
#endif
};

}  // namespace util
