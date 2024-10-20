#define BOOST_TEST_MODULE settings_test TestSuites
#define BOOST_TEST_DYN_LINK
#include <stdio.h>

#include <set>
#include <map>
#include <tuple>
#include <vector>
#include <queue>
#include <stack>
#include <list>
#include <boost/test/unit_test.hpp>
#include <cmath>
#include <limits>
#include <utils/templates/variant.hpp>

namespace util {


BOOST_AUTO_TEST_CASE(create_variants_test) {

  using MyVariant_0 = BaseTypeContainer<std::variant, false, int, double, float>::SingleTypeStl<std::set, std::vector>::PairedTypeStl<std::pair>::type;
  using MyTuples_0 = BaseTypeContainer<std::tuple, true, int, double, float>::SingleTypeStl<std::set, std::vector>::PairedTypeStl<std::pair>::type;

  using MyVariant_1 = BaseTypeContainer<std::variant, true, int, double, float>PairedTypeStl<std::map>::::SingleTypeStl<std::set, std::vector>::type;
  using MyVariant_2 = BaseTypeContainer<std::variant, true, int, double, float>PairedTypeStl<std::map, std::multi_map>::::SingleTypeStl<std::queue, std::stack>::type;

  std::vector<MyVariant_0> variants0;
  std::vector<MyTuples_0> tuples0;

  std::set<int> sampleSet({1,2,3,4});
  std::vector<double> sampleVector({1,2,3,4});
  std::list<float> sampleList({1,2,3,4});
  std::pair<int, double> samplePair(3, 4.4);

  variants0.push_back(sampleSet);
  variants0.push_back(sampleVector);
  variants0.push_back(sampleList);
  variants0.push_back(samplePair);

  tuples0.push_back(&sampleSet);
  tuples0.push_back(&sampleVector);
  tuples0.push_back(&sampleList);
  tuples0.push_back(&samplePair);

  // TODO BOOST_TEST();

  }

  }
