// gtest headers
#include <gtest/gtest.h>

// Task Tracker headers
#include "atom_feed.h"
#include "util.h"

TEST(TaskTracker, TestAtomFeed)
{
  std::ostringstream output;
  feed::WriteFeedXML(output);
  //std::cout<<"Feed: "<<std::endl;
  //std::cout<<output.str()<<std::endl;

  const size_t nMaxFileSizeBytes = 20 * 1024;
  std::string expected_output;
  EXPECT_TRUE(util::ReadFileIntoString("./test/data/feed.xml", nMaxFileSizeBytes, expected_output));
  EXPECT_FALSE(expected_output.empty());

  EXPECT_STREQ(expected_output.c_str(), output.str().c_str());
}
