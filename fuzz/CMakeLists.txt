# Set the minimum cmake version
CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

SET(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

project(fuzz_task_tracker)

add_compile_options(-std=c++23 -Wall -W -Wextra -Werror -Wformat -Wformat-y2k -Winit-self -Wstack-protector -Wunknown-pragmas -Wundef -Wwrite-strings -Wno-unused-parameter -Wpointer-arith -Wno-switch -Woverloaded-virtual -Wno-stack-protector -Wmissing-include-dirs -Wuninitialized -O2)

INCLUDE(FindPkgConfig)


INCLUDE_DIRECTORIES(../include/)
link_directories(../)

file(GLOB_RECURSE task_tracker_sources ../src/atom_feed.cpp ../src/curl_helper.cpp ../src/debug_fake_feed_entries_update_thread.cpp ../src/feed_data.cpp ../src/gitlab_api.cpp ../src/https_socket.cpp ../src/ip_address.cpp ../src/json.cpp ../src/random.cpp ../src/settings.cpp ../src/task_tracker.cpp ../src/task_tracker_thread.cpp ../src/util.cpp ../src/web_server.cpp ../src/xml_string_writer.cpp)

###############################################################################
## dependencies ###############################################################
###############################################################################

# libxml2
find_package(LibXml2 REQUIRED)

# libsecurityheaders library
set(SECURITYHEADERS_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/../../libsecurityheaders/include" CACHE STRING "libsecurityheaders include path")

# System microhttpd library
set(MicroHttpd_LIBRARY "-lmicrohttpd")

# System curl library
set(CURL_LIBRARY "-lcurl")
find_package(CURL REQUIRED)


# Fuzz Web Server HTTPS URL

ADD_EXECUTABLE(fuzz_web_server_https_url ${task_tracker_sources} ./src/fuzz_webserver_https_url.cpp ./src/gnutlsmm.cpp ./src/gnutlsmm_request.cpp ./src/tcp_connection.cpp)

target_compile_options(fuzz_web_server_https_url PRIVATE -fsanitize=address,fuzzer)
target_link_options(fuzz_web_server_https_url PRIVATE -fsanitize=address,fuzzer)


target_include_directories(fuzz_web_server_https_url SYSTEM PUBLIC include ${MICROHTTPD_INCLUDE_DIR} ${SECURITYHEADERS_INCLUDE_DIR} ${CURL_INCLUDE_DIR})
target_link_directories(fuzz_web_server_https_url PUBLIC ${MICROHTTPD_LIB_DIR} ${CURL_LIB_DIR})
target_link_libraries(fuzz_web_server_https_url PRIVATE -fsanitize=address,fuzzer gnutls gnutlsxx microhttpd json-c LibXml2::LibXml2 curl)

# Fuzz Web Server HTTPS Request

ADD_EXECUTABLE(fuzz_web_server_https_request ${task_tracker_sources} ./src/fuzz_webserver_https_request.cpp ./src/gnutlsmm.cpp ./src/gnutlsmm_request.cpp ./src/tcp_connection.cpp)

target_compile_options(fuzz_web_server_https_request PRIVATE -fsanitize=address,fuzzer)
target_link_options(fuzz_web_server_https_request PRIVATE -fsanitize=address,fuzzer)

target_include_directories(fuzz_web_server_https_request SYSTEM PUBLIC include ${MICROHTTPD_INCLUDE_DIR} ${SECURITYHEADERS_INCLUDE_DIR} ${CURL_INCLUDE_DIR})
target_link_directories(fuzz_web_server_https_request PUBLIC ${MICROHTTPD_LIB_DIR} ${CURL_LIB_DIR})
target_link_libraries(fuzz_web_server_https_request PRIVATE -fsanitize=address,fuzzer gnutls gnutlsxx microhttpd json-c LibXml2::LibXml2 curl)
