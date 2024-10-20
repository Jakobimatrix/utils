#define BOOST_TEST_MODULE settings_test TestSuites
#define BOOST_TEST_DYN_LINK
#include <stdio.h>

#include <boost/test/unit_test.hpp>
#include <cmath>
#include <limits>
#include <list>
#include <map>
#include <queue>
#include <set>
#include <stack>
#include <tuple>
#include <utils/templates/TypeContainer.hpp>
#include <variant>
#include <vector>

namespace util {


BOOST_AUTO_TEST_CASE(create_variants_test) {

  using MyVariant_0 =
      TypeContainerCreater<std::variant, false, int, double, float>::SingleTypeStl<stdVector, stdList>::PairedTypeStl<stdPair>::type;
  using MyTuples_0 =
      TypeContainerCreater<std::tuple, true, int, double, float>::SingleTypeStl<stdVector, stdList>::PairedTypeStl<stdPair>::type;

  using MyVariant_1 =
      TypeContainerCreater<std::variant, true, int, double, float>::PairedTypeStl<
          stdMap>::SingleTypeStl<stdSet, stdVector>::typeWithBase;
  using MyVariant_2 = TypeContainerCreater<std::variant, true, int, double, float>::
      PairedTypeStl<stdMap, stdMultimap>::SingleTypeStl<stdQueue, stdStack>::typeWithBase;

  std::vector<MyVariant_0> variants0;
  std::vector<const MyTuples_0> tuples0;

  bool sampleBool = true;
  int sampleInteger = 42;
  float sampleFloat = 3.14f;
  std::string sampleString = "Hello";

  std::vector<int> sampleVector = {1, 2, 3};
  std::map<int, std::string> sampleMap = {{1, "one"}, {2, "two"}};

  std::set<int> sampleSet({1, 2, 3, 4});
  std::list<float> sampleList({1, 2, 3, 4});
  std::pair<int, double> samplePair(3, 4.4);

  // variants0.push_back(sampleSet);
  variants0.push_back(sampleVector);
  variants0.push_back(sampleList);
  variants0.push_back(samplePair);

  // tuples0.push_back(&sampleSet);
  tuples0.push_back(&sampleVector);
  tuples0.push_back(&sampleList);
  tuples0.push_back(&samplePair);

  // TODO BOOST_TEST();
}

}  // namespace util
