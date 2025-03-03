#include <memory.h>

#include <chrono>
#include <fstream>
#include <iostream>
#include <span>
#include <sstream>
#include <string>
#include <thread>

#include "gnutlsmm.h"
#include "gnutlsmm_request.h"
#include "util.h"
#include "web_server.h"

namespace {

const util::cIPAddress host(127, 0, 0, 1);

}

namespace util {

uint64_t GetTimeMS()
{
  const std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
  auto ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now).time_since_epoch().count();
  return ms;
}

}


bool PerformHTTPSGetRequest(std::span<const char> const fuzz_data, uint16_t port, std::string_view server_certificate_path)
{
  std::cout<<"PerformHTTPSGetRequest"<<std::endl;

  std::string user_agent("FuzzTester");
  const std::string request(fuzz_data.data(), fuzz_data.size());
  if (!gnutlsmm::GnuTLSPerformRequest(host, port, request, user_agent, server_certificate_path)) {
    return true;
  }

  std::cout<<"PerformGetRequest returning true"<<std::endl;
  return true;
}

// This just fuzz tests the URL part of the HTTP request:
// "<fuzz data>"
// The fuzz data needs to be prompted with a corpus that starts off with some valid HTTP requests like:
// "GET / HTTP/1.0\r\n\r\n"
// "GET /not-found.txt HTTP/1.0\r\n\r\n"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t data_length)
{
  std::cout<<"LLVMFuzzerTestOneInput"<<std::endl;

  // libcurl can't cope with NULL or empty URLs
  if ((data == nullptr) || (data_length == 0)) {
    return -1;
  }

  gnutlsmm::helper gnutlsHelper;

  const uint16_t port = 10000 + (util::GetTimeMS() % 10000); // Pseudorandom port

  // Create the web server
  tasktracker::cWebServerManager web_server_manager;
  const bool fuzzing = true;
  if (!web_server_manager.Create(host, port, "../server.key", "../server.crt", fuzzing, "PJYM9sAlPgoeSDu5ekFC40Q3AFJMl1uidxivonEL1NZ3DXQzzP0D8uibgnvZxbkB")) {
    std::cerr<<"Error creating web server"<<std::endl;
    return -1;
  }

  while (!PerformHTTPSGetRequest(std::span<const char>{(const char*)data, data_length}, port, "../server.crt")) {
  }

  std::cout<<"Shutting down server"<<std::endl;
  if (!web_server_manager.Destroy()) {
    std::cerr<<"Error destroying web server"<<std::endl;
  }

  std::cout<<"LLVMFuzzerTestOneInput returning 0"<<std::endl;
  return 0;
}
