#pragma once

#include <chrono>
#include <string>
#include <vector>

#include "settings.h"

namespace gitlab {

class cIssue {
public:
  uint16_t iid;
  std::string title;
  std::chrono::system_clock::time_point due_date;
  std::string web_url;
};

bool QueryGitlabAPI(const tasktracker::cSettings& settings, std::vector<cIssue>& out_gitlab_issues);

}
