/**
 * @file variant.hpp
 * @brief Contains template to join multiple std::variant/std::tuple. Contains template to generate std::variant of std::map<Ti,Tj> all possible combinations.
 *
 * Detailed description, etc.
 */

#include <variant>
#include <utility>
#include <type_traits>

#pragma once

// joining variant types: https://stackoverflow.com/questions/64042612/join-the-types-of-stdvarianta-b-c-and-stdvariantx-y-z
/*
 * Joinst multiple std::variants<T1, T2> + std::variants<T3, T4> --> std::variants<T1, T2, T3, T4>
 * also works with std::tuple
 */
// Helper to concatenate multiple templates
template <class... Args>
struct template_concat;

template <template <class...> class T, class... Args1, class... Args2, typename... Remaining>
struct template_concat<T<Args1...>, T<Args2...>, Remaining...> {
  using type = typename template_concat<T<Args1..., Args2...>, Remaining...>::type;
};

template <template <class...> class T, class... Args1>
struct template_concat<T<Args1...>> {
  using type = T<Args1...>;
};

template <class... Args>
using template_concat_t = typename template_concat<Args...>::type;

// PointerWrapper: Adds pointer to type if `add_pointer` is true, otherwise leaves type unchanged.
template <typename T, bool add_pointer>
struct PointerWrapper {
  using type = T;  // No pointer
};

template <typename T>
struct PointerWrapper<T, true> {
  using type = T*;  // Adds pointer
};


// Specialization for two types, with optional pointer wrapping
template<template<typename, typename> class TemplateType, typename T1, typename T2, bool use_pointer>
struct generateTemplateVariants_T1_T2 {
  using type = std::variant<
      typename PointerWrapper<TemplateType<T1, T2>, use_pointer>::type,
      typename PointerWrapper<TemplateType<T2, T1>, use_pointer>::type
      >;
};

// Base case when no types remain
template<template<typename, typename> class TemplateType, bool use_pointer, typename... Ts>
struct generateTemplateVariantsForT1;

// Base case when no types remain
template<template<typename, typename> class TemplateType, bool use_pointer, typename T1>
struct generateTemplateVariantsForT1<TemplateType, use_pointer, T1> {
  using type = std::variant<>;  // No more combinations
};

// Generate pairs of TemplateType for T1 with all combinations of remaining types
template<template<typename, typename> class TemplateType, bool use_pointer, typename T1, typename T2, typename... Rest>
struct generateTemplateVariantsForT1<TemplateType, use_pointer, T1, T2, Rest...> {
  using type = typename template_concat<
      typename generateTemplateVariants_T1_T2<TemplateType, T1, T2, use_pointer>::type,
      typename generateTemplateVariantsForT1<TemplateType, use_pointer, T1, Rest...>::type
      >::type;
};


// General case for the template variant generation
template<template<typename, typename> class TemplateType, bool use_pointer, typename... Ts>
struct generateTemplateVariants;

template<template<typename, typename> class TemplateType, bool use_pointer, typename T>
struct generateTemplateVariants<TemplateType, use_pointer, T> {
  using type = std::variant<>;  // Base case, no combinations left
};

// Recursive generation of combinations for any TemplateType
template<template<typename, typename> class TemplateType, bool use_pointer, typename T1, typename T2, typename... Rest>
struct generateTemplateVariants<TemplateType, use_pointer, T1, T2, Rest...> {
  using VariantsForT1 = typename generateTemplateVariantsForT1<TemplateType, use_pointer, T1, T2, Rest...>::type;
  using VariantsForRest = typename generateTemplateVariants<TemplateType, use_pointer, T2, Rest...>::type;
  using type = template_concat_t<VariantsForT1, VariantsForRest>;
};

// Generate combinations of TemplateType applied to all pairs of types, with optional pointer wrapping
template <template<typename, typename> class TemplateType, bool use_pointer, typename... Types>
struct generateAllTemplateVariants {
  using SameTypesVariant = std::variant<typename PointerWrapper<TemplateType<Types, Types>, use_pointer>::type...>;
  using DifferentTypesVariant = typename generateTemplateVariants<TemplateType, use_pointer, Types...>::type;
  using type = template_concat_t<SameTypesVariant, DifferentTypesVariant>;
};

// Example usage with std::pair, std::map, without pointers
//using DataVariantPair = generateAllTemplateVariants<std::pair, false, bool, int, unsigned int, float, double, std::string>::type;
//using DataVariantMap = generateAllTemplateVariants<std::map, true, bool, int, unsigned int, float, double, std::string>::type;
// Example usage with std::pair, with pointers
//using DataVariantPairPointer = generateAllTemplateVariants<std::pair, true, bool, int, unsigned int, float, double, std::string>::type;




