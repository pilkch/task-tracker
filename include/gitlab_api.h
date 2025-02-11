#pragma once

#include <string>
#include <vector>

#include "settings.h"

namespace gitlab {

class cIssue {
public:
  uint16_t iid;
  std::string title;
  std::string due_date;
};

bool QueryGitlabAPI(const tasktracker::cSettings& settings, std::vector<cIssue>& out_gitlab_issues);

}
