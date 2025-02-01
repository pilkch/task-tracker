#include <cstring>
#include <functional>
#include <iostream>
#include <string>
#include <thread>

#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#include <sys/inotify.h>

#include "feed_data.h"
#include "poll_helper.h"
#include "task_tracker.h"
#include "task_tracker_thread.h"
#include "util.h"

namespace tasktracker {

void CheckTasksAndUpdateFeedEntries(cTaskList& tasks, uint64_t previous_update)
{/*
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


class cTaskTrackerThread {
public:
  cTaskTrackerThread();

  void MainLoop();

private:
  
};

cTaskTrackerThread::cTaskTrackerThread()
{
}


/* Size of buffer to use when reading inotify events */
#define INOTIFY_BUFFER_SIZE 8192

/* FANotify-like helpers to iterate events */
#define IN_EVENT_DATA_LEN (sizeof(struct inotify_event))
#define IN_EVENT_NEXT(event, length)            \
  ((length) -= (event)->len,                    \
   (struct inotify_event*)(((char *)(event)) +	\
                           (event)->len))
#define IN_EVENT_OK(event, length)                  \
  ((long)(length) >= (long)IN_EVENT_DATA_LEN &&	    \
   (long)(event)->len >= (long)IN_EVENT_DATA_LEN && \
   (long)(event)->len <= (long)(length))

namespace util {

class cFileWatcher {
public:
  cFileWatcher();
  ~cFileWatcher();

  void AddWatch(const std::string& file_path);

  int GetInotifyFD() const { return inotify_fd; }

  bool ReadEvents();

private:
  void RemoveWatch();

  int inotify_fd; // inotify file descriptor
  int monitor_fd; // Monitored file descriptor
};

cFileWatcher::cFileWatcher() :
  inotify_fd(-1),
  monitor_fd(-1)
{
  if ((inotify_fd = inotify_init()) < 0) {
    std::cerr<<"inotify_init failed "<<strerror(errno)<<std::endl;
  }
}

cFileWatcher::~cFileWatcher()
{
  RemoveWatch();

  close(inotify_fd);
  inotify_fd = -1;
}

void cFileWatcher::RemoveWatch()
{
  if ((inotify_fd != -1) && (monitor_fd != -1)) {
    inotify_rm_watch(inotify_fd, monitor_fd);
    monitor_fd = -1;
  }
}

void cFileWatcher::AddWatch(const std::string& file_path)
{
  RemoveWatch();

  // Setup inotify notifications mask
  const int event_mask =
    IN_CLOSE_WRITE |   // Writable File closed
    IN_MODIFY          // File modified
  ;

  if (inotify_fd != -1) {
    monitor_fd = inotify_add_watch(inotify_fd, file_path.c_str(), event_mask);
    if (monitor_fd < 0) {
      std::cerr<<"inotify_add_watch failed "<<strerror(errno)<<std::endl;
    }
  }
}

bool cFileWatcher::ReadEvents()
{
  bool result = false;

  // Read events from the inotify file descriptor
  char buffer[INOTIFY_BUFFER_SIZE];

  ssize_t length = read(inotify_fd, buffer, INOTIFY_BUFFER_SIZE);
  if (length > 0) {
    const struct inotify_event* event = (const struct inotify_event*)buffer;
    while (IN_EVENT_OK(event, length)) {
      // TODO: If we ever monitor more than one file or more than just "Did the user change the file", then we would need to actually process the event here, we currently just assume it is for the one file we are monitoring
      //ProcessEvent(event);
      result = true;
      event = IN_EVENT_NEXT(event, length);
    }
  }

  return result;
}

}

void cTaskTrackerThread::MainLoop()
{
  std::cout<<"cTaskTrackerThread::MainLoop"<<std::endl;

  cTaskList tasks;
  LoadTasksFromFile("./tasks.json", tasks);

  util::cFileWatcher file_watcher;
  file_watcher.AddWatch("./tasks.json");

  LoadStateFromFile("./state.json", feed_data);

  // TODO: Fix this
  uint64_t previous_update = 0;

  poll_read p(file_watcher.GetInotifyFD());

  while (true) {
    // TODO: Fix this
    const uint64_t time_until_next_update_ms = 5000;

    const POLL_READ_RESULT result = p.poll(time_until_next_update_ms);
    if (result == POLL_READ_RESULT::DATA_READY) {
      file_watcher.ReadEvents();

      std::cout<<"Loading tasks.json file"<<std::endl;
      if (!LoadTasksFromFile("./tasks.json", tasks)) {
        std::cerr<<"Error parsing tasks.json"<<std::endl;
      }
    }

    CheckTasksAndUpdateFeedEntries(tasks, previous_update);
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
  cTaskTrackerThread* pTaskTrackerThread = new cTaskTrackerThread();
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
