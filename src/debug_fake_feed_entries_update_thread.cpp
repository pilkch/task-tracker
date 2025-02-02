#include <cmath>

#include <functional>
#include <iostream>
#include <thread>

#include "atom_feed.h"
#include "feed_data.h"
#include "debug_fake_feed_entries_update_thread.h"
#include "util.h"

namespace tasktracker {

class cDebugFakeFeedEntriesUpdateThread {
public:
  void MainLoop();
};

void cDebugFakeFeedEntriesUpdateThread::MainLoop()
{
  std::cout<<"cDebugFakeFeedEntriesUpdateThread::MainLoop"<<std::endl;

  const std::vector<std::string> fake_entries = {
    "First entry",
    "Second entry",
    "Third entry",
    "Fourth entry",
    "Fifth entry",
    "Sixth entry",
    "Seventh entry",
    "Eighth entry",
    "Ninth entry",
    "Tenth entry"
  };
  const size_t n = 10;

  util::cPseudoRandomNumberGenerator rng;

  for (size_t i = 0; i < n; i++) {
    // Wait 5 seconds between adding each entry
    util::msleep(5000);

    cFeedEntry entry;
    entry.title = fake_entries[i];
    entry.summary = "This is a summary";
    entry.date_updated = util::GetTime();
    entry.id = feed::GenerateFeedID(rng);

    // Add this entry
    {
      std::lock_guard<std::mutex> lock(mutex_feed_data);
      tasktracker::feed_data.entries.push_back(entry);
    }
  }
}

// Not the most elegant method, but it works
int DebugFakeFeedEntriesUpdateRunThreadFunction(void* pData)
{
  if (pData == nullptr) {
    return 1;
  }

  cDebugFakeFeedEntriesUpdateThread* pThis = static_cast<cDebugFakeFeedEntriesUpdateThread*>(pData);
  if (pThis == nullptr) {
    return 1;
  }

  std::cout<<"DebugFakeFeedEntriesUpdateRunThreadFunction Calling MainLoop"<<std::endl;
  pThis->MainLoop();
  std::cout<<"DebugFakeFeedEntriesUpdateRunThreadFunction MainLoop returned"<<std::endl;

  return 0;
}

bool DebugStartFakeFeedEntriesUpdateThread()
{
  std::cout<<"DebugStartFakeFeedEntriesUpdateThread"<<std::endl;

  // Ok we have successfully connected and subscribed so now we can start the thread to read updates
  cDebugFakeFeedEntriesUpdateThread* pDebugFakeFeedEntriesUpdateThread = new cDebugFakeFeedEntriesUpdateThread;
  if (pDebugFakeFeedEntriesUpdateThread == nullptr) {
    std::cerr<<"DebugStartFakeFeedEntriesUpdateThread Error creating DebugFakeFeedEntriesUpdate thread, returning false"<<std::endl;
    return false;
  }

  // Start the thread
  // NOTE: We never release this, it is ugly, but we don't shut down gracefully. We could create a regular object, then give the thread a signal to stop, then have the thread exit gracefully
  std::thread* pThread = new std::thread(std::bind(&DebugFakeFeedEntriesUpdateRunThreadFunction, pDebugFakeFeedEntriesUpdateThread));
  (void)pThread;

  return true;
}

}
