// Application headers
#include "ring_buffer.h"

// gtest headers
#include <gtest/gtest.h>

TEST(Util, TestRingBuffer)
{
  // Test default constructor
  util::ring_buffer<int, 5> a;
  EXPECT_EQ(0, a.size());
  EXPECT_TRUE(a.empty());

  // Test pushing back
  a.push_back(1);
  a.push_back(2);
  EXPECT_EQ(2, a.size());
  EXPECT_TRUE(!a.empty());
  EXPECT_EQ(1, a[0]);
  EXPECT_EQ(2, a[1]);

  // Testing filling
  a.push_back(3);
  a.push_back(4);
  a.push_back(5);
  EXPECT_EQ(5, a.size());
  EXPECT_TRUE(!a.empty());
  EXPECT_EQ(1, a[0]);
  EXPECT_EQ(2, a[1]);
  EXPECT_EQ(3, a[2]);
  EXPECT_EQ(4, a[3]);
  EXPECT_EQ(5, a[4]);

  // Test filling more than the container size so that older entries are deleted
  a.push_back(6);
  a.push_back(7);
  a.push_back(8);
  EXPECT_EQ(5, a.size());
  EXPECT_TRUE(!a.empty());
  EXPECT_EQ(4, a[0]);
  EXPECT_EQ(5, a[1]);
  EXPECT_EQ(6, a[2]);
  EXPECT_EQ(7, a[3]);
  EXPECT_EQ(8, a[4]);
}
