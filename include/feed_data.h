#pragma once

#include <chrono>
#include <string>
#include <mutex>

#include "ring_buffer.h"

namespace tasktracker {

const size_t MAX_FEED_ENTRIES = 50;

class cFeedProperties {
public:
  std::string title;
  std::string link;
  std::chrono::system_clock::time_point date_updated;
  std::string author_name;
  std::string id;
};

class cFeedEntry {
public:
  std::string title;
  std::string link;
  std::string summary;
  std::chrono::system_clock::time_point date_updated; // NOTE: This is the date the event was published, not the task date due
  std::string id;
};

class cFeedData {
public:
  cFeedProperties properties;
  util::ring_buffer<cFeedEntry, MAX_FEED_ENTRIES> entries;
};


// Mutex and data
// Lock the mutex, use the data, and unlock the mutex
extern std::mutex mutex_feed_data;
extern cFeedData feed_data;

bool LoadFeedDataFromFile(const std::string& file_path, const std::string& external_url);
bool SaveFeedDataToFile(const std::string& file_path);

}
