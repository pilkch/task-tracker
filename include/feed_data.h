#pragma once

#include <list>
#include <string>
#include <mutex>

namespace tasktracker {

class cFeedEntry {
public:
  std::string title;
  std::string summary;
  uint64_t date_updated; // NOTE: This is the date the event was published, not the task date due
  std::string id;
};

class cFeedData {
public:
  std::list<cFeedEntry> entries;
};


// Mutex and data
// Lock the mutex, use the data, and unlock the mutex
extern std::mutex mutex_feed_data;
extern cFeedData feed_data;

bool LoadStateFromFile(const std::string& file_path, cFeedData& feed_data);

}
