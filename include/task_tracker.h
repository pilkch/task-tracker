#pragma once

#include <chrono>
#include <list>
#include <string>
#include <vector>

#include "settings.h"

namespace tasktracker {

class cTask {
public:
  std::string description;
  std::chrono::system_clock::time_point date_due;
};

class cTaskList {
public:
  std::vector<cTask> tasks;
};

bool LoadTasksFromFile(const std::string& file_path, cTaskList& tasks);

bool RunServer(const cSettings& settings);

}
