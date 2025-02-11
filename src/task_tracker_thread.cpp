#include <cstring>
#include <functional>
#include <iostream>
#include <string>
#include <thread>

#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#include "https_socket.h"
#include "json.h"
#include "feed_data.h"
#include "gitlab_api.h"
#include "poll_helper.h"
#include "task_tracker.h"
#include "task_tracker_thread.h"
#include "util.h"

namespace tasktracker {

class cTaskTrackerThread {
public:
  explicit cTaskTrackerThread(const cSettings& settings);

  void MainLoop();

private:
  void CheckTasksAndUpdateFeedEntries(cTaskList& tasks, std::chrono::system_clock::time_point previous_update);

  const cSettings& settings;
};

cTaskTrackerThread::cTaskTrackerThread(const cSettings& _settings) :
  settings(_settings)
{
}


void cTaskTrackerThread::CheckTasksAndUpdateFeedEntries(cTaskList& tasks, std::chrono::system_clock::time_point previous_update)
{
  // Query the gitlab API to get any tasks with expiry dates
  std::vector<gitlab::cIssue> out_gitlab_issues;
  gitlab::QueryGitlabAPI(settings, out_gitlab_issues);

  /*
  std::vector<cFeedEntry> entries_to_add;
  for (auto&& item : tasks.tasks) {
    // Check the date on each 
    if (date) {
      entries_to_add
    }
}

  // Update the feed entries
  std::lock_guard<std::mutex> lock(mutex_feed_data);
  for (auto&& item : entries_to_add) {
    // Add the entries

  }*/
}

void cTaskTrackerThread::MainLoop()
{
  std::cout<<"cTaskTrackerThread::MainLoop"<<std::endl;

  cTaskList tasks;
  LoadTasksFromFile("./tasks.json", tasks);

  LoadStateFromFile("./state.json", feed_data);

  // TODO: Fix this
  uint64_t uptime = 0;
  std::chrono::system_clock::time_point previous_update { std::chrono::duration_cast<std::chrono::system_clock::time_point::duration>(std::chrono::milliseconds(uptime)) };

  // We update on started up and then every hour after that
  const uint64_t minutes_between_updates = 60;

  while (true) {
    CheckTasksAndUpdateFeedEntries(tasks, previous_update);

    const uint64_t time_until_next_update_ms = minutes_between_updates * 60 * 1000;
    util::msleep(time_until_next_update_ms);
  }
}

// Not the most elegant method, but it works
int RunThreadFunction(void* pData)
{
  if (pData == nullptr) {
    return 1;
  }

  cTaskTrackerThread* pThis = static_cast<cTaskTrackerThread*>(pData);
  if (pThis == nullptr) {
    return 1;
  }

  std::cout<<"RunThreadFunction Calling MainLoop"<<std::endl;
  pThis->MainLoop();
  std::cout<<"RunThreadFunction MainLoop returned"<<std::endl;

  return 0;
}

bool StartTaskTrackerThread(const cSettings& settings)
{
  std::cout<<"StartTaskTrackerThread Running server "<<std::endl;

  // Ok we have successfully connected and subscribed so now we can start the thread to read updates
  cTaskTrackerThread* pTaskTrackerThread = new cTaskTrackerThread(settings);
  if (pTaskTrackerThread == nullptr) {
    std::cerr<<"StartTaskTrackerThread Error creating task tracker thread, returning false"<<std::endl;
    return false;
  }

  // Start the thread
  // NOTE: We never release this, it is ugly, but we don't shut down gracefully. We could create a regular object, then give the thread a signal to stop, then have the thread exit gracefully
  std::thread* pThread = new std::thread(std::bind(&RunThreadFunction, pTaskTrackerThread));
  (void)pThread;

  return true;
}

}
