/**
 * @file variadicFunction.hpp
 * @brief
 *
 * Detailed description, etc.
 */

#pragma once

#include <functional>
#include <tuple>

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
  /*!
   * \brief virtual function will be overwritten by children, to call the saved
   * function.
   */
  virtual void call() = 0;
};

template <typename... ARGS>
class VariadicFunction : public VirtualCall {
 public:
  /*!
   * \brief Constructor
   * provide the arguments and the pointer to the actual function.
   * \param args all arguments for the to be called function as std::tuple in
   * correct order.
   * \param Pointer to the to be called function.
   */
  VariadicFunction(std::tuple<ARGS...> args, void (*f)(ARGS...))
      : args(args), f(f) {}

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
    f(std::get<S>(args)...);
  }

 private:
  std::tuple<ARGS...> args;
  std::function<void(ARGS...)> f;
};
