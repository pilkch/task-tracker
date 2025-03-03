#include <cstring>
#include <functional>
#include <iostream>
#include <string>
#include <thread>

#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#include "atom_feed.h"
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
  void UpdateTaskListFromGitlabIssues(cTaskList& task_list);
  void AddFeedEntry(std::vector<cFeedEntry>& entries_to_add, const cTask& task, bool high_priority, const std::string& summary);
  void CheckTasksAndUpdateFeedEntries(cTaskList& task_list, const std::chrono::system_clock::time_point& start_time, const std::chrono::system_clock::time_point& end_time);

  const cSettings& settings;
  util::cPseudoRandomNumberGenerator rng;
};

cTaskTrackerThread::cTaskTrackerThread(const cSettings& _settings) :
  settings(_settings)
{
}

void cTaskTrackerThread::UpdateTaskListFromGitlabIssues(cTaskList& task_list)
{
  // Query the gitlab API to get any tasks with expiry dates
  std::vector<gitlab::cIssue> gitlab_issues;
  gitlab::QueryGitlabAPI(settings, gitlab_issues);

  // Add/update the tasks list
  for (auto&& issue : gitlab_issues) {
    cTask task;
    task.title = issue.title;
    task.date_due = issue.due_date;
    task.link = issue.web_url;

    task_list.tasks[issue.iid] = task;
  }
}

void cTaskTrackerThread::AddFeedEntry(std::vector<cFeedEntry>& entries_to_add, const cTask& task, bool high_priority, const std::string& summary)
{
  std::cout<<"Adding feed entry \""<<task.title<<"\": "<<summary<<std::endl;
  cFeedEntry entry;
  entry.title = (high_priority ? "🚩" : "🔔") + task.title;
  entry.summary = summary;
  entry.date_updated = util::GetTime();
  entry.id = feed::GenerateFeedID(rng);
  entry.link = task.link;

  entries_to_add.push_back(entry);
}

void cTaskTrackerThread::CheckTasksAndUpdateFeedEntries(cTaskList& task_list, const std::chrono::system_clock::time_point& start_time, const std::chrono::system_clock::time_point& end_time)
{
  UpdateTaskListFromGitlabIssues(task_list);

  // For each entry we need to check if it is getting close to the expiry date and we just passed a notification interval in the last update
  std::vector<cFeedEntry> entries_to_add;

  for (auto&& task : task_list.tasks) {
    // Check the date on each task
    if (util::IsDateWithinRange(task.second.date_due - std::chrono::weeks(3), start_time, end_time)) {
      AddFeedEntry(entries_to_add, task.second, false, "Task is due in 3 weeks");
    } else if (util::IsDateWithinRange(task.second.date_due - std::chrono::weeks(1), start_time, end_time)) {
      AddFeedEntry(entries_to_add, task.second, false, "Task is due in 1 week");
    } else if (util::IsDateWithinRange(task.second.date_due - std::chrono::days(1), start_time, end_time)) {
      AddFeedEntry(entries_to_add, task.second, true, "Task is due in 1 day");
    } else if (util::IsDateWithinRange(task.second.date_due, start_time, end_time)) {
      AddFeedEntry(entries_to_add, task.second, true, "Task is due now!");
    }
  }

  if (!entries_to_add.empty()) {
    {
      // Update the feed entries
      std::lock_guard<std::mutex> lock(mutex_feed_data);
      feed_data.entries.push_back(std::span<cFeedEntry>(entries_to_add));
    }

    SaveFeedDataToFile();
  }
}

void cTaskTrackerThread::MainLoop()
{
  std::cout<<"cTaskTrackerThread::MainLoop"<<std::endl;

  cTaskList task_list;
  LoadTasksFromFile("./tasks.json", task_list);

  // NOTE: task-trackerd wants to be running 24/7. It will miss whatever events happen when it is not running, it doesn't remember the last time it did an update and will not catch up on missed events.
  std::chrono::system_clock::time_point previous_update = util::GetTime();

  // We update soon after start up and then every half an hour after that
  const uint64_t minutes_between_updates = 30;

  // Sleep for 30 seconds before the first update
  util::msleep(30 * 1000);

  while (true) {
    const std::chrono::system_clock::time_point start_time = previous_update;
    const std::chrono::system_clock::time_point end_time = util::GetTime();
    CheckTasksAndUpdateFeedEntries(task_list, start_time, end_time);
    previous_update = end_time;

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
