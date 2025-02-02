// gtest headers
#include <gtest/gtest.h>

// Task Tracker headers
#include "atom_feed.h"
#include "util.h"

namespace {

void AddFeedItem(tasktracker::cFeedData& feed_data, util::cPseudoRandomNumberGenerator& rng, const std::string& link, const std::string& title, const std::string& summary, std::chrono::system_clock::time_point time)
{
  tasktracker::cFeedEntry entry;
  entry.title = title;
  entry.link = link;
  entry.summary = summary;
  entry.date_updated = time;
  entry.id = feed::GenerateFeedID(rng);

  feed_data.entries.push_back(entry);
}

}

TEST(TaskTracker, TestAtomFeed)
{
  tasktracker::cFeedData feed_data;

  const uint32_t seed = 12345;
  util::cPseudoRandomNumberGenerator rng(seed);

  feed_data.properties.title = "Example Feed";
  feed_data.properties.link = "http://example.org/";
  feed_data.properties.date_updated = std::chrono::time_point<std::chrono::system_clock>(std::chrono::milliseconds(1738496275130));
  feed_data.properties.author_name = "John Doe";
  feed_data.properties.id = feed::GenerateFeedID(rng);

  const std::chrono::system_clock::time_point time1(std::chrono::milliseconds(1738497894544));
  AddFeedItem(feed_data, rng, "http://example.org/2003/12/13/my-first-entry", "Item 1 Title", "Item 1 summary", time1);
  const std::chrono::system_clock::time_point time2(std::chrono::milliseconds(1738495489349));
  AddFeedItem(feed_data, rng, "http://example.org/2003/12/14/my-second-entry", "Item 2 Title", "Item 2 summary", time2);

  std::ostringstream output;
  feed::WriteFeedXML(feed_data, output);
  //std::cout<<"Feed: "<<std::endl;
  //std::cout<<output.str()<<std::endl;

  const size_t nMaxFileSizeBytes = 20 * 1024;
  std::string expected_output;
  EXPECT_TRUE(util::ReadFileIntoString("./test/data/feed.xml", nMaxFileSizeBytes, expected_output));
  EXPECT_FALSE(expected_output.empty());

  EXPECT_STREQ(expected_output.c_str(), output.str().c_str());
}
