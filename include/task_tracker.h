#pragma once

#include <chrono>
#include <string>
#include <map>

#include "settings.h"

namespace tasktracker {

class cTask {
public:
  std::string title;
  std::chrono::system_clock::time_point date_due;
};

class cTaskList {
public:
  std::map<uint16_t, cTask> tasks; // Map of iid (Gitlab unique issue ID) to task
};

bool LoadTasksFromFile(const std::string& file_path, cTaskList& tasks);

bool RunServer(const cSettings& settings);

}
