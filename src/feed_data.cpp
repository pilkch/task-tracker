#include "feed_data.h"

namespace tasktracker {

std::mutex mutex_feed_data;
cFeedData feed_data;

bool LoadStateFromFile(const std::string& file_path, cFeedData& feed_data)
{
  return true;
}

}
