#include <ctime>

#include <iostream>
#include <sstream>

#include "gitlab_api.h"
#include "https_socket.h"
#include "json.h"

namespace gitlab {

inline bool ParseGitlabIssuesDateTimeYY_MM_SS(const std::string& buffer, std::chrono::system_clock::time_point& value) noexcept
{
  // Parse a date like "2024-04-01"
  std::istringstream ss(buffer);
  std::tm tm{};
  if (!(ss >> std::get_time(&tm, "%Y-%m-%d"))) {
    std::cerr<<"ParseGitlabIssuesDateTimeYY_MM_SS failed"<<std::endl;
    return false;
  }

  value = std::chrono::system_clock::time_point{std::chrono::seconds(std::mktime(&tm))};
  return true;
}

bool ParseGitlabIssuesResponse(const std::string& response, std::vector<cIssue>& out_gitlab_issues)
{
  json::cJSONDocument document(json_tokener_parse(response.c_str()));
  if (!document.IsValid()) {
    std::cerr<<"ParseGitlabIssuesResponse Invalid JSON response"<<std::endl;
    return false;
  }

  // Parse the JSON tree

  enum json_type root_type = json_object_get_type(document.Get());
  if (root_type != json_type_array) {
    std::cerr<<"ParseGitlabIssuesResponse Root node is not an array"<<std::endl;
    return false;
  }

  // Parse the root level array
  const size_t n = json_object_array_length(document.Get());
	for (size_t i = 0; i < n; i++) {
    // Parse an issue
		struct json_object* issue = json_object_array_get_idx(document.Get(), i);
    enum json_type issue_type = json_object_get_type(issue);
    if (issue_type != json_type_object) {
      std::cerr<<"ParseGitlabIssuesResponse Issue found that is not an object"<<std::endl;
      return false;
    }

    cIssue new_issue;

    if (!json::JSONParseUint16(issue, "iid", new_issue.iid)) {
      return false;
    }

    if (!json::JSONParseString(issue, "title", new_issue.title)) {
      return false;
    }

    std::string value;
    if (!json::JSONParseString(issue, "due_date", value)) {
      return false;
    }

    if (!ParseGitlabIssuesDateTimeYY_MM_SS(value, new_issue.due_date)) {
      return false;
    }

    if (!json::JSONParseString(issue, "web_url", new_issue.web_url)) {
      return false;
    }

    //std::cout<<"Item: "<<new_issue.iid<<", "<<new_issue.title<<", "<<new_issue.due_date<<", "<<web_url<<std::endl;
    out_gitlab_issues.push_back(new_issue);
  }

  return true;
}

bool QueryGitlabIssuesAPI(const tasktracker::cSettings& settings, const std::string& due_date, std::vector<cIssue>& out_gitlab_issues)
{
  // Perform a HTTP request something like this:
  // curl --insecure --header "PRIVATE-TOKEN: $GITLAB_ACCESS_TOKEN" "https://<my server>:2443/api/v4/groups/home/issues?state=opened&due_date=next_month_and_previous_two_weeks" | jq -r '.[].iid' | sort

  // https://docs.gitlab.com/ee/api/issues.html#list-group-issues
  // due_date can be one of today, tomorrow, overdue, week, month, next_month_and_previous_two_weeks
  const std::string URL = settings.GetGitlabURL() + "api/v4/groups/home/issues?state=opened&due_date=" + due_date;

  curl::cHTTPSSocket socket;
  if (!socket.Open(URL, settings.GetGitlabHTTPSPublicCert(), settings.GetGitlabAPIToken())) {
    std::cerr<<"QueryGitlabIssuesAPI Error connecting to server"<<std::endl;
    return false;
  }

  std::ostringstream o;
  if (!socket.ReadToString(o)) {
    std::cerr<<"QueryGitlabIssuesAPI Error querying gitlab API"<<std::endl;
    return false;
  }

  //std::cout<<"QueryGitlabIssuesAPI returned \""<<o.str()<<"\""<<std::endl;

  // Parse JSON response
  ParseGitlabIssuesResponse(o.str(), out_gitlab_issues);

  return true;
}

bool QueryGitlabAPI(const tasktracker::cSettings& settings, std::vector<cIssue>& out_gitlab_issues)
{
  out_gitlab_issues.clear();

  return QueryGitlabIssuesAPI(settings, "next_month_and_previous_two_weeks", out_gitlab_issues);
}

}
