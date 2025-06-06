/**
 * @file TypeContainer.hpp
 * @brief Contains template to join multiple std::variant/std::tuple.
 * Contains template to generate a type container (std::variant/tuple) of std::map<Ti,Tj>/pair<Ti,Tj>... all possible combinations.
 * Contains template to generate type container (std::variant/tuple) of all std::container, given the base types.
 *
 * @version 1.0
 * @date 2021
 */

#include <type_traits>
#include <utility>

#include <map>
#include <map>
#include <set>
#include <unordered_map>
#include <queue>
#include <vector>
#include <list>
#include <stack>
#include <unordered_set>

#pragma once

namespace util{

// joining variant types: https://stackoverflow.com/questions/64042612/join-the-types-of-stdvarianta-b-c-and-stdvariantx-y-z
/*
 * Joinst multiple std::variants<T1, T2> + std::variants<T3, T4> -->
 * std::variants<T1, T2, T3, T4> also works with std::tuple
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
  using type = T *;  // Adds pointer
};

template <typename T, bool add_pointer>
using PointerWrapper_t = typename PointerWrapper<T, add_pointer>::type;


// Specialization for two types, with optional pointer wrapping
template <template <typename...> class TypeContainer, template <typename, typename> class TemplateType, typename T1, typename T2, bool use_pointer>
struct generateTemplateVariants_T1 {
  using type =
      TypeContainer<typename PointerWrapper<TemplateType<T1, T2>, use_pointer>::type,
                    typename PointerWrapper<TemplateType<T2, T1>, use_pointer>::type>;
};

// Base case when no types remain
template <template <typename...> class TypeContainer, template <typename, typename> class TemplateType, bool use_pointer, typename... Ts>
struct generateTemplateVariantsForT1;

// Base case when no types remain
template <template <typename...> class TypeContainer, template <typename, typename> class TemplateType, bool use_pointer, typename T1>
struct generateTemplateVariantsForT1<TypeContainer, TemplateType, use_pointer, T1> {
  using type = TypeContainer<>;  // No more combinations
};

// Generate pairs of TemplateType for T1 with all combinations of remaining types
template <template <typename...> class TypeContainer, template <typename, typename> class TemplateType, bool use_pointer, typename T1, typename T2, typename... Rest>
struct generateTemplateVariantsForT1<TypeContainer, TemplateType, use_pointer, T1, T2, Rest...> {
  using type = typename template_concat<
      typename generateTemplateVariants_T1<TypeContainer, TemplateType, T1, T2, use_pointer>::type,
      typename generateTemplateVariantsForT1<TypeContainer, TemplateType, use_pointer, T1, Rest...>::type>::type;
};


// General case for the template variant generation
template <template <typename...> class TypeContainer, template <typename, typename> class TemplateType, bool use_pointer, typename... Ts>
struct generateTemplateVariants;

template <template <typename...> class TypeContainer, template <typename, typename> class TemplateType, bool use_pointer, typename T>
struct generateTemplateVariants<TypeContainer, TemplateType, use_pointer, T> {
  using type = TypeContainer<>;  // Base case, no combinations left
};

// Recursive generation of combinations for any TemplateType
template <template <typename...> class TypeContainer, template <typename, typename> class TemplateType, bool use_pointer, typename T1, typename T2, typename... Rest>
struct generateTemplateVariants<TypeContainer, TemplateType, use_pointer, T1, T2, Rest...> {
  using VariantsForT1 =
      typename generateTemplateVariantsForT1<TypeContainer, TemplateType, use_pointer, T1, T2, Rest...>::type;
  using VariantsForRest =
      typename generateTemplateVariants<TypeContainer, TemplateType, use_pointer, T2, Rest...>::type;
  using type = template_concat_t<VariantsForT1, VariantsForRest>;
};

/*
Wrapper Explanation:
  In C++, most STL containers like std::set, std::map, etc., have default template parameters, such as the comparator for std::set (std::less<T>) and the allocator for most containers.
  GCC allows you to pass the container without explicitly providing all template parameters, but Clang requires that the template signature matches exactly.
  To avoid this discrepancy, you can define wrappers that explicitly match the expected number of template parameters.
*/
template <typename T>
using stdSet = std::set<T>;

template <typename T>
using stdVector = std::vector<T>;

template <typename T>
using stdList = std::list<T>;

template <typename T>
using stdDeque = std::deque<T>;

template <typename T>
using stdQueue = std::queue<T>;

template <typename T>
using stdPriorityQueue = std::priority_queue<T>;

template <typename T>
using stdStack = std::stack<T>;

template <typename T>
using stdUnorderedSet = std::unordered_set<T>;

template <typename T, typename U>
using stdUnorderedMap = std::unordered_map<T, U>;

template <typename T, typename U>
using stdMap = std::map<T, U>;

template <typename T, typename U>
using stdMultimap = std::multimap<T, U>;

template <typename T, typename U>
using stdUnorderedMultimap = std::unordered_multimap<T, U>;

template <typename T>
using stdMultiset = std::multiset<T>;

template <typename T>
using stdUnorderedMultiset = std::unordered_multiset<T>;

template <typename T, typename U>
using stdPair = std::pair<T, U>;

/*Generate combinations of TemplateType applied to all pairs of types, with optional pointer wrapping
// Example usage with std::pair, std::map, without pointers
// using DataVariantPair = generateAllPairedTemplateVariants<std::variant, stdPair, false, bool, int, unsigned int, float, double, std::string>::type;
// using DataTupleMap = generateAllPairedTemplateVariants<std::tuple, stdMap, false, bool, int, unsigned int, float, double, std::string>::type;
// Example usage with std::pair, with pointers
// using DataVariantPairPointer = generateAllPairedTemplateVariants<std::variant, std::pair, true, bool, int, unsigned int, float, double, std::string>::type;
*/
template <template <typename...> class TypeContainer, template <typename, typename> class TemplateType, bool use_pointer, typename... Types>
struct generateAllPairedTemplateVariants {
  using SameTypesVariant =
      TypeContainer<typename PointerWrapper<TemplateType<Types, Types>, use_pointer>::type...>;
  using DifferentTypesVariant =
      typename generateTemplateVariants<TypeContainer, TemplateType, use_pointer, Types...>::type;
  using type = template_concat_t<SameTypesVariant, DifferentTypesVariant>;
};


// using MyVariant_0 = TypeContainerCreater<std::variant, false, int, double, float>::SingleTypeStl<std::set, std::vector>::PairedTypeStl<std::pair>::type;
// using MyVariant_1 = TypeContainerCreater<std::tuple, false, int, double, float>::PairedTypeStl<std::map>::SingleTypeStl<std::list>::typeWithBase;
// using MyVariant_2 = TypeContainerCreater<std::variant, true, int, double, float, unsigned long>::PairedTypeStl<std::unordered_map>::type;
template <template <typename...> class TypeContainer, bool use_pointer, typename... BaseTypes>
struct TypeContainerCreater {
  using BaseVariantsType = TypeContainer<PointerWrapper_t<BaseTypes, use_pointer>...>;

  template <template <typename> class... StlContainers>
  struct SingleStlTypeContainer;

  template <template <typename> class StlContainer>
  struct SingleStlTypeContainer<StlContainer> {
    using type = TypeContainer<PointerWrapper_t<StlContainer<BaseTypes>, use_pointer>...>;
  };

  template <template <typename> class StlContainer, template <typename> class... OtherStlContainers>
  struct SingleStlTypeContainer<StlContainer, OtherStlContainers...> {
    using type =
        template_concat_t<typename SingleStlTypeContainer<StlContainer>::type,
                          typename SingleStlTypeContainer<OtherStlContainers...>::type>;
  };


  template <template <typename, typename> class... StlContainers_Paired>
  struct PairedStlTypeContainer;

  template <template <typename, typename> class StlContainer_Paired>
  struct PairedStlTypeContainer<StlContainer_Paired> {
    using type = typename
        generateAllPairedTemplateVariants<TypeContainer, StlContainer_Paired, use_pointer, BaseTypes...>::type;
  };

  template <template <typename, typename> class StlContainer_Paired, template <typename, typename> class... OtherStlContainers_Paired>
  struct PairedStlTypeContainer<StlContainer_Paired, OtherStlContainers_Paired...> {
    using type =
        template_concat_t<typename PairedStlTypeContainer<StlContainer_Paired>::type,
                          typename PairedStlTypeContainer<OtherStlContainers_Paired...>::type>;
  };

  template <template <typename> class... StlContainers>
  struct SingleTypeStl {
    template <template <typename, typename> class... StlContainers_Paired>
    struct PairedTypeStl {
      using type =
          template_concat_t<typename SingleStlTypeContainer<StlContainers...>::type,
                            typename PairedStlTypeContainer<StlContainers_Paired...>::type>;
      using typeWithBase = template_concat_t<BaseVariantsType, type>;
    };
    using type = typename SingleStlTypeContainer<StlContainers...>::type;
    using typeWithBase = template_concat_t<BaseVariantsType, type>;
  };

  template <template <typename, typename> class... StlContainers_Paired>
  struct PairedTypeStl {
    template <template <typename> class... StlContainers>
    struct SingleTypeStl {
      using type =
          template_concat_t<typename PairedStlTypeContainer<StlContainers_Paired...>::type,
                            typename SingleStlTypeContainer<StlContainers...>::type>;
      using typeWithBase = template_concat_t<BaseVariantsType, type>;
    };
    using type = typename PairedStlTypeContainer<StlContainers_Paired...>::type;
    using typeWithBase = template_concat_t<BaseVariantsType, type>;
  };
};

}  // namespace util
