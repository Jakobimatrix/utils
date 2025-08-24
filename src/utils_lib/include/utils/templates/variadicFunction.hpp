/**
 * @file variadicFunction.hpp
 * @brief Contains a template class to call a function with variadic arguments.
 *
 * This file defines a `VariadicFunction` class that allows storing a function
 * with variadic arguments and calling it later without needing to pass the
 * arguments again. It uses a tuple to store the arguments and a virtual base
 * class to enable polymorphic behavior.
 *
 * @version 2.0
 * @date 2025.08.24
 */

#pragma once

#include <cstddef>
#include <functional>
#include <tuple>
#include <utility>

namespace util {

/*!
 * \brief Virtual base function to be able to store a pointer to different
 * templated children.
 */
class VirtualCall {
 public:
  VirtualCall() noexcept;

  /*!
   * \brief virtual function will be overwritten by children, to call the saved
   * function.
   */
  virtual void call() = 0;

  // Needs to be public because I delete base pointers std::unique_ptr<VirtualCall>
  virtual ~VirtualCall() noexcept;
  // rule of 5
  VirtualCall(const VirtualCall&)                = default;
  VirtualCall& operator=(const VirtualCall&)     = default;
  VirtualCall(VirtualCall&&) noexcept            = default;
  VirtualCall& operator=(VirtualCall&&) noexcept = default;
};

template <typename... ARGS>
class VariadicFunction : public VirtualCall {
 public:
  /*!
   * \brief Constructor
   * provide the arguments and the pointer to the actual function.
   * \param func Pointer to the to be called function.
   * \param args all arguments for the to be called function as std::tuple in
   * correct order.
   */
  template <typename... Ts>
  VariadicFunction(std::function<void(ARGS...)> func, Ts&&... args)
      : arguments(std::forward_as_tuple(std::forward<Ts>(args)...)),
        varFunc(std::move(func)) {}

  ~VariadicFunction() noexcept override = default;
  // rule of 5
  VariadicFunction(const VariadicFunction&)                = default;
  VariadicFunction& operator=(const VariadicFunction&)     = default;
  VariadicFunction(VariadicFunction&&) noexcept            = default;
  VariadicFunction& operator=(VariadicFunction&&) noexcept = default;

  /*!
   * \brief To call the saved function without the need of arguments.
   */
  void call() override { callFunc(std::index_sequence_for<ARGS...>{}); }

  /*!
   * \brief To call the saved function.
   * Helper to unpack the tuple into the function call.
   */
  template <std::size_t... I>
  void callFunc(std::index_sequence<I...> /*unused*/) {
    varFunc(std::get<I>(arguments)...);
  }

 private:
  std::tuple<ARGS...> arguments;
  std::function<void(ARGS...)> varFunc;
};

}  // namespace util
