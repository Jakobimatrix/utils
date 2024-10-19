

#pragma once

// joining variant types: https://stackoverflow.com/questions/64042612/join-the-types-of-stdvarianta-b-c-and-stdvariantx-y-z
/*
 * Joinst multiple std::variants<T1, T2> + std::variants<T3, T4> --> std::variants<T1, T2, T3, T4>
 * also works with std::tuple
 */
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


/*
 * given an list of Template Types, generate std::variant<map<T1,T2>, map<T1,T3>, ..., map<T1,TN>, ... map<Tn-1,Tn>>
 * All possible combinations, but not where key and value are both same Tx!!!
 */
template<typename... Ts>
struct generateMapVariants_T1;

// generate std::variant<std::map<T1,T2>, std::map<T2,T1>>
template<typename T1, typename T2>
struct generateMapVariants_T1<T1, T2> {
  using type = std::variant<
    std::map<T1, T2>*,
    std::map<T2, T1>*
    >;
};

// generates std::variant<std::map<T1,T2>, std::map<T1,T3>, ..., std::map<T1,Tn>>
// generates std::variant<std::map<T2,T1>, std::map<T3,T1>, ..., std::map<Tn,T1>>
// all pairs with T1
template<typename T1, typename T2, typename... Rest>
struct generateMapVariants_T1<T1, T2, Rest...> {
  using VariantsT1T2 = typename generateMapVariants_T1<T1, T2>::type;
  using VariantsT1Rest = typename generateMapVariants_T1<T1, Rest...>::type;
  using type = template_concat_t<VariantsT1T2, VariantsT1Rest>;
};

template<typename... Ts>
struct generateMapVariants;

template<typename T>
struct generateMapVariants<T> {
  using type = std::variant<>; // only gets called with the last entry
};

// calls for each T generateMapVariants_T1 with Tx and the Rest Tx+1...Tn
template<typename T1, typename... Rest>
struct generateMapVariants<T1, Rest...> {
  using VariantsT1 = typename generateMapVariants_T1<T1, Rest...>::type;
  using VariantsTRest = typename generateMapVariants<Rest...>::type;
  using type = template_concat_t<VariantsT1, VariantsTRest>;
};

// creates std::variant<T1*, ..., Tn*, std::vector<T1>*, ..., std::vector<Tn>*, std::map<>* (all combinations)>
template <typename... Types>
struct CombinedVariant {
  using BaseTypesAndVectorAndMapSingleTypeVariant = std::variant<Types*..., std::vector<Types>*..., std::map<Types,Types>*...>;
  using MapDifferentTypesVariant = typename generateMapVariants<Types...>::type;

  using type = template_concat_t<BaseTypesAndVectorAndMapSingleTypeVariant, MapDifferentTypesVariant>;
};

// Example usage with specific types
//using DataVariant = CombinedVariant<bool, int, unsigned int, float, double, std::string>::type;





template <class T>
void printX(std::vector<T>* vec){
  std::cout << "vector:";
  for(const auto& val : *vec){
    std::cout << val << std::endl;
  }
  std::cout << std::endl;
}

template <class T1, class T2>
void printX(std::map<T1, T2>* map){
  std::cout << "map:";
  for(const auto& [key, value] :  *map){
    std::cout << "{" << key << ", " << value << "}\n";
  }
  std::cout << std::endl;
}

template <class T>
void printX(T* value){
  std::cout << "value: "  << *value << "\n";
}


main(){
using DataVariant = CombinedVariant<bool, int, unsigned int, float, double, std::string>::type;

// Create variables of basic types
bool b = true;
int i = 42;
unsigned int ui = 100;
float f = 3.14f;
double d = 2.718281828;
std::string s = "Hello";

// Create vectors
std::vector<int> vec1 = {1, 2, 3, 4, 5};
std::vector<std::string> vec2 = {"A", "B", "C"};

// Create maps
std::map<int, std::string> map1 = {{1, "one"}, {2, "two"}};
std::map<std::string, double> map2 = {{"pi", 3.14159}, {"e", 2.71828}};

// Create a vector of MyTypes variants and fill it
std::vector<DataVariant> variants;
variants.push_back(&b);
variants.push_back(&i);
variants.push_back(&ui);
variants.push_back(&f);
variants.push_back(&d);
variants.push_back(&s);
variants.push_back(&vec1);
variants.push_back(&vec2);
variants.push_back(&map1);
variants.push_back(&map2);


// Iterate over variants and print the content
for (const auto& variant : variants) {
  std::visit([](auto&& arg) {
    //using T = std::decay_t<std::remove_pointer<decltype(arg)>>;
    printX(arg);
  }, variant);
}

}
