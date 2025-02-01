#include <fstream>
#include <iostream>
#include <string>

#include "task_tracker.h"
#include "util.h"
#include "web_server.h"

// Enable this to turn on a debug mode where we don't read from the AC UDP socket, and instead just cycle the RPM and speed up and down for testing purposes
#define DEBUG_FAKE_FEED_ENTIES

#ifdef DEBUG_FAKE_FEED_ENTIES
#include "debug_fake_feed_entries_update_thread.h"
#else
#include "task_tracker_thread.h"
#endif

namespace tasktracker {

bool LoadTasksFromFile(const std::string& file_path, cTaskList& tasks)
{
  return true;
}

bool RunServer(const cSettings& settings)
{
  std::cout<<"Running server"<<std::endl;

#ifndef DEBUG_FAKE_FEED_ENTIES
  // Start the task tracker thread
  if (!StartTaskTrackerThread(settings)) {
    std::cerr<<"Error starting task tracker"<<std::endl;
    return false;
  }
#else
  // Start the DebugStartFakeFeedEntriesUpdateThread thread for debugging
  if (!DebugStartFakeFeedEntriesUpdateThread()) {
    std::cerr<<"Error creating DebugStartFakeFeedEntriesUpdateThread"<<std::endl;
    return false;
  }
#endif

  // Now run the web server
  cWebServerManager web_server_manager;
  if (!web_server_manager.Create(settings.GetHTTPSHost(), settings.GetHTTPSPort(), settings.GetHTTPSPrivateKey(), settings.GetHTTPSPublicCert())) {
    std::cerr<<"Error creating web server"<<std::endl;
    return false;
  }

  if (settings.GetRunningInContainer()) {
    while (true) {
      util::msleep(500);
    }
  } else {
    std::cout<<"Press enter to shutdown the server"<<std::endl;
    (void)getc(stdin);
  }

  std::cout<<"Shutting down server"<<std::endl;
  if (!web_server_manager.Destroy()) {
    std::cerr<<"Error destroying web server"<<std::endl;
    return false;
  }

  std::cout<<"Server has been shutdown"<<std::endl;
  return true;
}

}
