#include "gnutlsmm.h"

// gtest headers
#include <gtest/gtest.h>

int main(int argc, char** argv)
{
  gnutlsmm::helper gnutlsHelper;

  ::testing::InitGoogleTest(&argc, argv);

  //testing::GTEST_FLAG(filter) = "Util.TestRingBuffer";
  //testing::GTEST_FLAG(filter) = "TaskTracker.TestAtomFeed";

  const int result = RUN_ALL_TESTS();

  return result;
}
