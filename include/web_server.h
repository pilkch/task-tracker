#pragma once

#include "ip_address.h"

namespace tasktracker {

class cStaticResourcesRequestHandler;
class cDynamicResourcesRequestHandler;
class cWebServer;

// TODO: Refactor this, it is a bit of a mess
class cWebServerManager {
public:
  cWebServerManager();
  ~cWebServerManager();

  bool Create(const util::cIPAddress& host, uint16_t port, const std::string& private_key, const std::string& public_cert, bool fuzzing, const std::string& token);
  bool Destroy();

private:
  // NOTE: We would use std::unique_ptr, but it needs to know about the destructor of the item to delete it
  cStaticResourcesRequestHandler* static_resources_request_handler;
  cDynamicResourcesRequestHandler* dynamic_resources_request_handler;
  cWebServer* webserver;
};

}
