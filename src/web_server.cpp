#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdint>
#include <cstring>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <fstream>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include <arpa/inet.h>
#include <fcntl.h>

#include <microhttpd.h>

#include <security_headers.h>

#include "atom_feed.h"
#include "feed_data.h"
#include "util.h"
#include "web_server.h"

// For "ms" literal suffix
using namespace std::chrono_literals;

namespace {

const std::string HTML_MIMETYPE = "text/html";
const std::string CSS_MIMETYPE = "text/css";
const std::string SVG_XML_MIMETYPE = "image/svg+xml";
const std::string ATOM_FEED_MIMETYPE = "application/rss+xml";

}

namespace tasktracker {

class cStaticResource {
public:
  void Clear();

  std::string request_path;
  std::string response_mime_type;
  std::string response_text;
};


const std::string UNAUTHORISED = "401 Unauthorized";
const std::string PAGE_NOT_FOUND = "404 Not Found";



void ServerAddSecurityHeaders(struct MHD_Response* response)
{
  security_headers::policy p;
  p.strict_transport_security_max_age_days = 365;
  static const auto&& headers = security_headers::GetSecurityHeaders(p);

  for (auto&& header : headers) {
    MHD_add_response_header(response, header.name.c_str(), header.value.c_str());
  }
}

bool Server401Unauthorised(struct MHD_Connection* connection)
{
  struct MHD_Response* response = MHD_create_response_from_buffer_static(UNAUTHORISED.length(), UNAUTHORISED.c_str());
  const enum MHD_Result ret = MHD_queue_response(connection, MHD_HTTP_UNAUTHORIZED, response);
  const char* mime = nullptr;
  MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_ENCODING, mime);
  ServerAddSecurityHeaders(response);
  MHD_destroy_response(response);
  return (ret == MHD_YES);;
}

enum MHD_Result Server404NotFoundResponse(struct MHD_Connection* connection)
{
  struct MHD_Response* response = MHD_create_response_from_buffer_static(PAGE_NOT_FOUND.length(), PAGE_NOT_FOUND.c_str());
  const enum MHD_Result ret = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
  const char* mime = nullptr;
  MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_ENCODING, mime);
  ServerAddSecurityHeaders(response);
  MHD_destroy_response(response);
  return ret;
}

bool ServerRegularResponse(struct MHD_Connection* connection, std::string_view content, std::string_view mime_type)
{
  // NOTE: content needs to be long lived, static, libmicrohttpd keeps a reference to it
  struct MHD_Response* response = MHD_create_response_from_buffer_static(content.length(), content.data());
  MHD_add_response_header(response, "Content-Type", mime_type.data());
  ServerAddSecurityHeaders(response);
  const int result = MHD_queue_response(connection, MHD_HTTP_OK, response);
  MHD_destroy_response(response);
  return (result == MHD_YES);
}

bool ServerRegularDynamicResponse(struct MHD_Connection* connection, std::string_view content, std::string_view mime_type)
{
  // NOTE: We have to use MHD_RESPMEM_MUST_COPY so that libmicrohttpd makes a copy of the content
  struct MHD_Response* response = MHD_create_response_from_buffer(content.length(), (void*)content.data(), MHD_RESPMEM_MUST_COPY);
  MHD_add_response_header(response, "Content-Type", mime_type.data());
  ServerAddSecurityHeaders(response);
  const int result = MHD_queue_response(connection, MHD_HTTP_OK, response);
  MHD_destroy_response(response);
  return (result == MHD_YES);
}

}

namespace tasktracker {

class cStaticResourcesRequestHandler {
public:
  bool LoadStaticResources();

  bool HandleRequest(struct MHD_Connection* connection, std::string_view url);

private:
  bool LoadStaticResource(const std::string& request_path, const std::string& response_mime_type, const std::string& file_path);

  // Yes, this could have been a map of std::string to std::pair<std::string, std::string>
  std::vector<cStaticResource> static_resources;
};

bool cStaticResourcesRequestHandler::LoadStaticResource(const std::string& request_path, const std::string& response_mime_type, const std::string& file_path)
{
  cStaticResource resource;

  const size_t nMaxFileSizeBytes = 20 * 1024;
  if (!util::ReadFileIntoString(file_path, nMaxFileSizeBytes, resource.response_text)) {
    std::cerr<<"File \""<<file_path<<"\" not found"<<std::endl;
    return false;
  }

  resource.request_path = request_path;
  resource.response_mime_type = response_mime_type;

  static_resources.push_back(resource);

  return true;
}

bool cStaticResourcesRequestHandler::LoadStaticResources()
{
  static_resources.clear();

  // A bit of a hack, this will find the resources folder either from the main folder, or from the fuzz folder
  const std::string resources_folder = util::TestFolderExists("resources") ? "./resources" : "../resources";

  return (
    LoadStaticResource("/", HTML_MIMETYPE, resources_folder + "/index.html") &&
    LoadStaticResource("/style.css", CSS_MIMETYPE, resources_folder + "/style.css") &&
    LoadStaticResource("/favicon.svg", SVG_XML_MIMETYPE, resources_folder + "/favicon.svg")
  );
}

bool cStaticResourcesRequestHandler::HandleRequest(struct MHD_Connection* connection, std::string_view url)
{
  // Handle static resources
  if (!url.empty() && (url[0] == '/')) {
    for (auto&& resource : static_resources) {
      if (url == resource.request_path) {
        // This is the requested resource so create a response
        std::cout<<"Serving: 200 \""<<url<<"\" static"<<std::endl;
        return ServerRegularResponse(connection, resource.response_text, resource.response_mime_type);
      }
    }
  }

  return false;
}


class cDynamicResourcesRequestHandler {
public:
  explicit cDynamicResourcesRequestHandler(const std::string& token);

  bool HandleRequest(struct MHD_Connection* connection, std::string_view url);

private:
  bool IsTokenMatch(const char* user_token) const;

  std::string expected_token;
};

cDynamicResourcesRequestHandler::cDynamicResourcesRequestHandler(const std::string& token) :
  expected_token(token)
{
}

bool cDynamicResourcesRequestHandler::IsTokenMatch(const char* user_token) const
{
  if (user_token == nullptr) return false;

  const std::string user_token_str(user_token);

  return (user_token_str == expected_token);
}

bool cDynamicResourcesRequestHandler::HandleRequest(struct MHD_Connection* connection, std::string_view url)
{
  // Handle dynamic resources
  if (url == "/feed/atom.xml") {
    const char* user_token = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "token");
    if (!IsTokenMatch(user_token)) {
      std::cout<<"Serving: 401 \""<<url<<"\" dynamic"<<std::endl;
      return Server401Unauthorised(connection);
    } else {
      // The user has supplied the expected token, show the feed
      std::ostringstream output;
      {
        std::mutex mutex_feed_data;
        feed::WriteFeedXML(feed_data, output);
      }

      const std::string content = output.str();

      // This is the requested resource so create a response
      std::cout<<"Serving: 200 \""<<url<<"\" dynamic"<<std::endl;
      return ServerRegularDynamicResponse(connection, content, ATOM_FEED_MIMETYPE);
    }
  }

  return false;
}

}


namespace tasktracker {

class cWebServer {
public:
  cWebServer(cStaticResourcesRequestHandler& static_resources_request_handler, cDynamicResourcesRequestHandler& dynamic_resources_request_handler);
  ~cWebServer();

  bool Open(const util::cIPAddress& host, uint16_t port, const std::string& private_key, const std::string& public_cert, bool fuzzing);
  void NoMoreConnections();
  bool Close();

private:
  static enum MHD_Result _OnRequest(
    void* cls,
    struct MHD_Connection* connection,
    const char* url,
    const char* method,
    const char* version,
    const char* upload_data,
    size_t* upload_data_size,
    void** req_cls
  );

  struct MHD_Daemon* daemon;

  cStaticResourcesRequestHandler& static_resources_request_handler;
  cDynamicResourcesRequestHandler& dynamic_resources_request_handler;
};

cWebServer::cWebServer(cStaticResourcesRequestHandler& _static_resources_request_handler, cDynamicResourcesRequestHandler& _dynamic_resources_request_handler) :
  daemon(nullptr),
  static_resources_request_handler(_static_resources_request_handler),
  dynamic_resources_request_handler(_dynamic_resources_request_handler)
{
}

cWebServer::~cWebServer()
{
  NoMoreConnections();
  Close();
}

bool cWebServer::Open(const util::cIPAddress& host, uint16_t port, const std::string& private_key, const std::string& public_cert, bool fuzzing)
{
  const std::string address(util::ToString(host));

  struct sockaddr_in sad;
  memset(&sad, 0, sizeof(sad));
  if (inet_pton(AF_INET, address.c_str(), &(sad.sin_addr.s_addr)) != 1) {
    std::cerr<<"V4 inet_pton fail for "<<address<<std::endl;
    return false;
  }

  sad.sin_family = AF_INET;
  sad.sin_port   = htons(port);

  std::vector<struct MHD_OptionItem> options = {
    { MHD_OPTION_CONNECTION_TIMEOUT, static_cast<intptr_t>((unsigned int)120), nullptr },
    { MHD_OPTION_SOCK_ADDR, reinterpret_cast<intptr_t>((const struct sockaddr*)&sad), nullptr },
  };

  if (fuzzing) {
    options.push_back({ MHD_OPTION_LISTENING_ADDRESS_REUSE, static_cast<intptr_t>(1), nullptr }); // So that we can bind the port repeatedly in quick succession
  } else {
    options.push_back({ MHD_OPTION_CONNECTION_LIMIT, static_cast<intptr_t>(10), nullptr }); // Rate limit  to 100 simultaneous connections per IP
  }


  if (!private_key.empty() && !public_cert.empty()) {
    std::cout<<"cWebServer::Run Starting server at https://"<<address<<":"<<port<<"/"<<std::endl;
    std::string server_key;
    util::ReadFileIntoString(private_key, 10 * 1024, server_key);
    std::string server_cert;
    util::ReadFileIntoString(public_cert, 10 * 1024, server_cert);

    options.push_back({ MHD_OPTION_HTTPS_MEM_KEY, 0, static_cast<void*>(const_cast<char*>(server_key.c_str())) });
    options.push_back({ MHD_OPTION_HTTPS_MEM_CERT, 0, static_cast<void*>(const_cast<char*>(server_cert.c_str())) });

    options.push_back({ MHD_OPTION_END, 0, nullptr });

    daemon = MHD_start_daemon(MHD_USE_AUTO
                          | MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_ERROR_LOG
                          | MHD_USE_TLS,
                          port,
                          nullptr, nullptr,
                          &_OnRequest, this,
                          MHD_OPTION_ARRAY,
                          options.data(),
                          MHD_OPTION_END);
  } else {
    std::cout<<"cWebServer::Run Starting server at http://"<<address<<":"<<port<<"/"<<std::endl;

    options.push_back({ MHD_OPTION_END, 0, nullptr });

    daemon = MHD_start_daemon(MHD_USE_AUTO
                          | MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_ERROR_LOG,
                          port,
                          nullptr, nullptr,
                          &_OnRequest, this,
                          MHD_OPTION_ARRAY,
                          options.data(),
                          MHD_OPTION_END);
  }

  return (daemon != nullptr);
}

void cWebServer::NoMoreConnections()
{
  if (daemon != nullptr) {
    MHD_quiesce_daemon(daemon);
  }
}

bool cWebServer::Close()
{
  // Stop the server
  if (daemon != nullptr) {
    MHD_stop_daemon(daemon);
    daemon = nullptr;
  }

  return true;
}



/**
 * Function called by the MHD_daemon when the client tries to access a page.
 *
 * This is used to provide html, css, javascript, images, and icons.
 *
 * @param cls closure, whatever was given to #MHD_start_daemon().
 * @param connection The HTTP connection handle
 * @param url The requested URL
 * @param method The request method (typically "GET")
 * @param version The HTTP version
 * @param upload_data Given upload data for POST requests
 * @param upload_data_size The size of the upload data
 * @param req_cls A pointer for request specific data
 * @return MHD_YES on success or MHD_NO on error.
 */
enum MHD_Result cWebServer::_OnRequest(
  void* cls,
  struct MHD_Connection* connection,
  const char* url,
  const char* method,
  const char* version,
  const char* upload_data,
  size_t* upload_data_size,
  void** req_cls
)
{
  //std::cout<<"cWebServer::_OnRequest "<<url<<std::endl;
  (void)upload_data;
  (void)upload_data_size;

  static int aptr = 0;

  if (0 != strcmp(method, "GET")) {
    return MHD_NO;              /* unexpected method */
  }

  if (&aptr != *req_cls) {
    // Never respond on first call
    *req_cls = &aptr;
    return MHD_YES;
  }
  // We have handled a request before
  *req_cls = nullptr;


  cWebServer* pThis = static_cast<cWebServer*>(cls);
  if (pThis == nullptr) {
    std::cerr<<"Error pThis is NULL"<<std::endl;
    return MHD_NO;
  }

  // Handle static resources
  if (pThis->static_resources_request_handler.HandleRequest(connection, url)) {
    return MHD_YES;
  }

  // Handle dynamic resources
  if (pThis->dynamic_resources_request_handler.HandleRequest(connection, url)) {
    return MHD_YES;
  }

  // Unknown resource
  std::cout<<"Serving: 404 \""<<url<<"\""<<std::endl;
  return Server404NotFoundResponse(connection);
}


cWebServerManager::cWebServerManager() :
  static_resources_request_handler(nullptr),
  dynamic_resources_request_handler(nullptr),
  webserver(nullptr)
{
}

cWebServerManager::~cWebServerManager()
{
  if (webserver != nullptr) {
    delete webserver;
    webserver = nullptr;
  }

  if (static_resources_request_handler != nullptr) {
    delete static_resources_request_handler;
    static_resources_request_handler = nullptr;
  }

  if (dynamic_resources_request_handler != nullptr) {
    delete dynamic_resources_request_handler;
    dynamic_resources_request_handler = nullptr;
  }
}

bool cWebServerManager::Create(const util::cIPAddress& host, uint16_t port, const std::string& private_key, const std::string& public_cert, bool fuzzing, const std::string& token)
{
  if (
    (static_resources_request_handler != nullptr) ||
    (dynamic_resources_request_handler != nullptr) ||
    (webserver != nullptr)
  ) {
    std::cerr<<"Error already created"<<std::endl;
    return false;
  }

  static_resources_request_handler = new cStaticResourcesRequestHandler;

  // Load the static resources
  if (!static_resources_request_handler->LoadStaticResources()) {
    return false;
  }

  dynamic_resources_request_handler = new cDynamicResourcesRequestHandler(token);

  webserver = new cWebServer(*static_resources_request_handler, *dynamic_resources_request_handler);
  if (!webserver->Open(host, port, private_key, public_cert, fuzzing)) {
    std::cerr<<"Error opening web server"<<std::endl;
    return false;
  }

  std::cout<<"Server is running"<<std::endl;

  return true;
};

bool cWebServerManager::Destroy()
{
  std::cout<<"Shutting down the server"<<std::endl;

  webserver->NoMoreConnections();

  // Wait for the connection threads to respond
  std::cout<<"Waiting for the connection threads to respond"<<std::endl;
  sleep(2);

  /* usually we should wait here in a safe way for all threads to disconnect, */
  /* but we skip this in the example */

  webserver->Close();

  return true;
}

}
