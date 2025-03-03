#include <cstdlib>

#include <sstream>

#include <unistd.h>

#include "https_socket.h"

namespace {

static size_t write_data(void* buffer, size_t size, size_t nmemb, void* user_data)
{
  if (user_data == nullptr) return -1;

  // Write the data to our file
  std::ostringstream* pOutput = (std::ostringstream*)user_data;
  pOutput->write((const char*)buffer, size * nmemb);
  return (size * nmemb);
}

}


namespace curl {

cHTTPSSocket::cHTTPSSocket() :
  curl(nullptr)
{
}

cHTTPSSocket::~cHTTPSSocket()
{
  Close();
}

bool cHTTPSSocket::Open(std::string_view url, const std::optional<std::string>& self_signed_certificate_path, const std::string& api_token)
{
  Close();

  curl = curl_easy_init();
  if (curl == nullptr) {
    return false;
  }

  curl_easy_setopt(curl, CURLOPT_URL, url.data());
  curl_easy_setopt(curl, CURLOPT_USERAGENT, url.data());
  //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L); // Turn on debugging

  curl_easy_setopt(curl, CURLOPT_XOAUTH2_BEARER, api_token.c_str());
  curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BEARER); // Tell curl to use the bearer above

  // Check if we need to provide a self signed public key
  if (self_signed_certificate_path) {
    curl_easy_setopt(curl, CURLOPT_CAINFO, self_signed_certificate_path.value().c_str());
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1);
  }

  return true;
}

bool cHTTPSSocket::ReadToString(std::ostringstream& output)
{
  // Send all data to this function
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);

  // Set the user data for the write function
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &output);

  // Perform the request
  const CURLcode result = curl_easy_perform(curl);

  return (result == CURLE_OK);
}

void cHTTPSSocket::Close()
{
  curl_easy_cleanup(curl);
  curl = nullptr;
}

}
