#include <climits>
#include <cstring>

#include <limits>
#include <iostream>
#include <filesystem>

#include <pwd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <json-c/json.h>

#include "json.h"
#include "settings.h"
#include "util.h"

namespace tasktracker {

cSettings::cSettings() :
  running_in_container(false),
  port(0)
{
}

bool cSettings::LoadFromFile(const std::string& sFilePath)
{
  Clear();

  const size_t nMaxFileSizeBytes = 20 * 1024;
  std::string contents;
  if (!util::ReadFileIntoString(sFilePath, nMaxFileSizeBytes, contents)) {
    std::cerr<<"File \""<<sFilePath<<"\" not found"<<std::endl;
    return false;
  }

  json::cJSONDocument document(json_tokener_parse(contents.c_str()));
  if (!document.IsValid()) {
    std::cerr<<"Invalid JSON config \""<<sFilePath<<"\""<<std::endl;
    return false;
  }

  // Parse the JSON tree

  // Parse "settings"
  json_object_object_foreach(document.Get(), settings_key, settings_val) {
    enum json_type type_settings = json_object_get_type(settings_val);
    if ((type_settings != json_type_object) || (strcmp(settings_key, "settings") != 0)) {
      std::cerr<<"settings object not found"<<std::endl;
      return false;
    }

    // Parse running in container (Optional)
    {
      bool value = 0;
      if (json::JSONParseBool(settings_val, "running_in_container", value)) {
        running_in_container = value;
      }
    }

    // Parse https address
    {
      std::string value;
      if (!json::JSONParseString(settings_val, "ip", value)) {
        return false;
      }

      util::ParseAddress(value, ip);
    }

    // Parse https port
    {
      uint16_t value = 0;
      if (!json::JSONParseUint16(settings_val, "port", value)) {
        return false;
      }

      port = value;
    }

    // Parse external URL
    if (!json::JSONParseString(settings_val, "external_url", external_url)) {
      return false;
    }

    // Parse https private key and certificate
    if (!json::JSONParseString(settings_val, "https_private_key", https_private_key)) {
      return false;
    }

    if (!json::JSONParseString(settings_val, "https_public_cert", https_public_cert)) {
      return false;
    }

    // Parse token
    if (!json::JSONParseString(settings_val, "token", token)) {
      return false;
    }


    // Parse gitlab settings
    if (!json::JSONParseString(settings_val, "gitlab_url", gitlab_url)) {
      return false;
    }

    if (!json::JSONParseString(settings_val, "gitlab_api_token", gitlab_api_token)) {
      return false;
    }

    if (!json::JSONParseString(settings_val, "gitlab_https_public_cert", gitlab_https_public_cert)) {
      return false;
    }
  }

  return IsValid();
}

constexpr bool cSettings::IsValid() const
{
  return (
    (ip.IsValid() || (util::ToString(ip) == "0.0.0.0")) && (port != 0) &&
    !external_url.empty() &&
    !https_private_key.empty() && !https_public_cert.empty() &&
    !token.empty() &&
    !gitlab_url.empty() && !gitlab_api_token.empty() && !gitlab_https_public_cert.empty()
  );
}

void cSettings::Clear()
{
  running_in_container = false;
  ip.Clear();
  port = 0;
  external_url.clear();
  https_private_key.clear();
  https_public_cert.clear();
  token.clear();
  gitlab_url.clear();
  gitlab_api_token.clear();
  gitlab_https_public_cert.clear();
}

}
