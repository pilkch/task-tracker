#pragma once

#include <cstdint>
#include <random>

namespace util {

// ** cPseudoRandomNumberGenerator
//
// Similar to using srand and rand
//
class cPseudoRandomNumberGenerator {
public:
  cPseudoRandomNumberGenerator()
  {
    generator.seed(static_cast<unsigned int>(time(nullptr)));
  }

  explicit cPseudoRandomNumberGenerator(uint32_t seed)
  {
    generator.seed(seed);
  }

  inline uint32_t random(uint32_t maximum)
  {
    return generator() % maximum;
  }

private:
  std::minstd_rand generator;
};

}
