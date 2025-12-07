/**
 * @file variadicFunction.cpp
 * @brief implementation for the VirtualCall class to avoid leaking into every translation unit -Wweak-vtables.
 * @date 29.05.2025
 * @author Jakob Wandel
 **/

#include <utils/templates/variadicFunction.hpp>

namespace util {


VirtualCall::VirtualCall() noexcept  = default;
VirtualCall::~VirtualCall() noexcept = default;

}  // namespace util
