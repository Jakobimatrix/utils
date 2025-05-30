#include <utils/system/memory.hpp>

#if defined(_WIN32) || defined(_WIN64)  // Windows
#include <windows.h>
#include <psapi.h>

#elif defined(__linux__)  // Linux
#include <fstream>
#include <string>
#include <sstream>
#elif defined(__APPLE__)  // macOS
#include <mach/mach.h>
#endif

namespace util {

double MemoryUsage::get(MemoryUnit unit) {
#if defined(_WIN32) || defined(_WIN64)
  const double memoryUsageInBytes = static_cast<double>(getMemoryUsageWindows());
#elif defined(__linux__)
  const double memoryUsageInBytes = static_cast<double>(getMemoryUsageLinux());
#elif defined(__APPLE__)
  const double memoryUsageInBytes = static_cast<double>(getMemoryUsageMac());
#else
  throw std::runtime_error("Unsupported platform");
#endif
  switch (unit) {
    case MemoryUnit::B:
      return memoryUsageInBytes;
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

#if defined(_WIN32) || defined(_WIN64)
std::size_t MemoryUsage::getMemoryUsageWindows() {
  PROCESS_MEMORY_COUNTERS memInfo;
  // Use K32GetProcessMemoryInfo for compatibility with modern Windows.
  if (K32GetProcessMemoryInfo(GetCurrentProcess(), &memInfo, sizeof(memInfo))) {
    return memInfo.WorkingSetSize;  // Bytes
  }
  return 0;
}
#elif defined(__linux__)
std::size_t MemoryUsage::getMemoryUsageLinux() {
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
std::size_t MemoryUsage::getMemoryUsageMac() {
  mach_task_basic_info info;
  mach_msg_type_number_t count = MACH_TASK_BASIC_INFO_COUNT;
  if (task_info(mach_task_self(), MACH_TASK_BASIC_INFO, (task_info_t)&info, &count) == KERN_SUCCESS) {
    return info.resident_size;  // Bytes
  }
  return 0;
}
#endif

}  // namespace util
