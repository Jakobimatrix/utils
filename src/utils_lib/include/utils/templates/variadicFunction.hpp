/**
 * @file variadicFunction.hpp
 * @brief Contains a template class to call a function with variadic arguments.
 *
 * This file defines a `VariadicFunction` class that allows storing a function
 * with variadic arguments and calling it later without needing to pass the
 * arguments again. It uses a tuple to store the arguments and a virtual base
 * class to enable polymorphic behavior.
 *
 * @version 1.0
 * @date 2021
 */

#pragma once

#include <functional>
#include <tuple>

namespace util {

// inspired by http://www.perry.cz/clanky/functions.html
template <int... Is>
struct seq {};

template <int N, int... Is>
struct gen_seq2 : gen_seq2<N - 1, N - 1, Is...> {};

template <int... S>
struct gen_seq2<0, S...> {
  typedef seq<S...> type;
};

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
};

template <typename... ARGS>
class VariadicFunction : public VirtualCall {
 public:
  /*!
   * \brief Constructor
   * provide the arguments and the pointer to the actual function.
   * \param args all arguments for the to be called function as std::tuple in
   * correct order.
   * \param f Pointer to the to be called function.
   */
  VariadicFunction(std::tuple<ARGS...> args, void (*f)(ARGS...))
      : arguments(args),
        varFunc(f) {}

  virtual ~VariadicFunction() noexcept override = default;

  /*!
   * \brief To call the saved function without the need of arguments.
   */
  void call() override { callFunc(typename gen_seq2<sizeof...(ARGS)>::type()); }

  /*!
   * \brief To call the saved function.
   * seq The index-list of the arguments for the function call.
   */
  template <int... S>
  void callFunc(seq<S...>) {
    varFunc(std::get<S>(arguments)...);
  }

 private:
  std::tuple<ARGS...> arguments;
  std::function<void(ARGS...)> varFunc;
};

}  // namespace util
