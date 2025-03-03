#pragma once

#include <cstdint>
#include <string>

#include "ip_address.h"

namespace tasktracker {

class cSettings {
public:
  cSettings();

  bool LoadFromFile(const std::string& file_path);

  constexpr bool IsValid() const;
  void Clear();

  constexpr bool GetRunningInContainer() const { return running_in_container; }

  constexpr const util::cIPAddress& GetIP() const { return ip; }
  constexpr uint16_t GetPort() const { return port; }
  constexpr const std::string& GetExternalURL() const { return external_url; }
  constexpr const std::string& GetHTTPSPrivateKey() const { return https_private_key; }
  constexpr const std::string& GetHTTPSPublicCert() const { return https_public_cert; }
  constexpr const std::string& GetToken() const { return token; }

  constexpr const std::string& GetGitlabURL() const { return gitlab_url; }
  constexpr const std::string& GetGitlabAPIToken() const { return gitlab_api_token; }
  constexpr const std::string& GetGitlabHTTPSPublicCert() const { return gitlab_https_public_cert; }

private:
  bool running_in_container;

  // RSS server settings
  util::cIPAddress ip;
  uint16_t port;
  std::string external_url;
  std::string https_private_key;
  std::string https_public_cert;
  std::string token;

  // Gitlab settings
  std::string gitlab_url;
  std::string gitlab_api_token;
  std::string gitlab_https_public_cert;
};

}
