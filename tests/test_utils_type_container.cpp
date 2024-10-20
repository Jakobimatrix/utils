#define BOOST_TEST_MODULE settings_test TestSuites
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <cmath>
#include <limits>
#include <list>
#include <map>
#include <queue>
#include <set>
#include <stack>
#include <tuple>
#include <variant>
#include <vector>

//
#include <utils/templates/TypeContainer.hpp>

namespace util {



BOOST_AUTO_TEST_CASE(create_variants_test) {
  // Test 1: Basic variant with single-type STL containers
  using MyVariant_0 =
      TypeContainerCreater<std::variant, false, int, double, float>::SingleTypeStl<stdVector, stdList>::PairedTypeStl<stdPair>::type;

  std::vector<MyVariant_0> variants0;
  std::vector<int> sampleVector = {1, 2, 3};
  std::list<float> sampleList = {4.5f, 5.6f, 6.7f};
  std::pair<int, double> samplePair(10, 20.5);

  // Force instantiations of MyVariant_0 with different types
  variants0.push_back(sampleVector);
  variants0.push_back(sampleList);
  variants0.push_back(samplePair);

  // Verify storage and retrieval of values from variants
  BOOST_TEST(std::get<std::vector<int>>(variants0[0]) == sampleVector);
  BOOST_TEST(std::get<std::list<float>>(variants0[1]) == sampleList);

  using ActualType = decltype(std::get<9>(variants0[2]));
  // static_assert(std::is_same_v<ActualType, std::pair<int, double>>, "Type mismatch");

  BOOST_TEST((std::get<std::pair<int, double>>(variants0[2]) == samplePair));
  // BOOST_TEST(std::get<std::pair<int, double>>(variants0[2]) == samplePair);

  // Test 2: Adding more combinations with single and paired containers
  using MyVariant_1 =
      TypeContainerCreater<std::variant, true, int, double, float>::PairedTypeStl<
          stdMap>::SingleTypeStl<stdSet, stdVector>::typeWithBase;

  std::vector<MyVariant_1> variants1;
  std::set<int> sampleSet({1, 2, 3, 4});
  std::map<int, double> sampleMap({{1, 1.1}, {2, 2.2}});

  variants1.push_back(&sampleSet);
  variants1.push_back(&sampleMap);

  // Verify storage and retrieval
  BOOST_TEST(*std::get<std::set<int>*>(variants1[0]) == sampleSet);
  BOOST_TEST(*(std::get<std::map<int, double>*>(variants1[1])) == sampleMap);

  // Test 3: Testing tuples with the same container combinations
  using MyTuples_0 =
      TypeContainerCreater<std::tuple, true, int, double, float>::SingleTypeStl<stdVector, stdList>::PairedTypeStl<stdPair>::type;

  std::tuple<std::vector<int>, std::list<float>, std::pair<int, double>> myTuple =
      std::make_tuple(sampleVector, sampleList, samplePair);

  // Access elements of the tuple
  BOOST_TEST(std::get<0>(myTuple) == sampleVector);
  BOOST_TEST(std::get<1>(myTuple) == sampleList);
  BOOST_TEST(std::get<2>(myTuple).first == samplePair.first);
  BOOST_TEST(std::get<2>(myTuple).second == samplePair.second);

  // Test 4: Variant with multiple paired containers
  using MyVariant_2 = TypeContainerCreater<std::variant, true, int, double, float>::
      PairedTypeStl<stdMap, stdMultimap>::SingleTypeStl<stdQueue, stdStack>::typeWithBase;

  std::vector<MyVariant_2> variants2;
  std::map<int, double> sampleMap2({{3, 3.3}, {4, 4.4}});
  std::multimap<int, float> sampleMultimap({{5, 5.5f}, {6, 6.6f}});
  std::queue<int> sampleQueue;
  std::stack<float> sampleStack;

  double sampleDouble(42.24);

  sampleQueue.push(1);
  sampleQueue.push(2);
  sampleStack.push(10.1f);
  sampleStack.push(20.2f);

  variants2.push_back(&sampleMap2);
  variants2.push_back(&sampleMultimap);
  variants2.push_back(&sampleQueue);
  variants2.push_back(&sampleStack);
  variants2.push_back(&sampleDouble);

  // Verify storage and retrieval
  BOOST_TEST(*(std::get<std::map<int, double>*>(variants2[0])) == sampleMap2);
  BOOST_TEST(*(std::get<std::multimap<int, float>*>(variants2[1])) == sampleMultimap);
  BOOST_TEST(*(std::get<double*>(variants2[4])) == sampleDouble);

  // Special checks for queue and stack (since direct equality doesn't work with std::queue/std::stack)
  BOOST_TEST(std::get<std::queue<int>*>(variants2[2])->front() == 1);
  BOOST_TEST(std::get<std::stack<float>*>(variants2[3])->top() == 20.2f);

  // Test 5: Empty type tests and ensuring the default types are handled
  using MyVariant_3 =
      TypeContainerCreater<std::variant, false, int>::SingleTypeStl<stdVector>::type;
  MyVariant_3 emptyVariant = std::vector<int>{};

  BOOST_TEST(std::get<std::vector<int>>(emptyVariant).empty());
}

BOOST_AUTO_TEST_CASE(create_tuple_test) {
  using MyTuple_0 =
      TypeContainerCreater<std::tuple, false, int, double, float>::SingleTypeStl<stdVector>::type;

  // Sample values to store in the tuple
  std::vector<int> sampleVectorI = {
      1,
      2,
      3,
  };
  std::vector<double> sampleVectorD = {1., 2., 3.};
  std::vector<float> sampleVectorF = {1.f, 2.f, 3.f};


  // Create a tuple containing different types
  MyTuple_0 myTuple = std::make_tuple(sampleVectorI, sampleVectorD, sampleVectorF);

  // Verify retrieval of values from the tuple
  BOOST_TEST(std::get<0>(myTuple) == sampleVectorI);
  BOOST_TEST(std::get<1>(myTuple) == sampleVectorD);
  BOOST_TEST(std::get<2>(myTuple) == sampleVectorF);
}

}  // namespace util
