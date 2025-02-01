#pragma once

#include <list>
#include <string>
#include <vector>

#include "settings.h"

namespace tasktracker {

class cTask {
public:
  std::string description;
  uint64_t date_due;
};

class cTaskList {
public:
  std::vector<cTask> tasks;
};

bool LoadTasksFromFile(const std::string& file_path, cTaskList& tasks);

bool RunServer(const cSettings& settings);

}
