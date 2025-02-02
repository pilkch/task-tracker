#pragma once

#include <cstdint>

#include <chrono>
#include <string_view>

namespace util {

template <class T>
inline constexpr T clamp(const T& i, const T& lower, const T& upper)
{
  return (i < lower) ? lower : (i > upper) ? upper : i;
}

// msleep(): Sleep for the requested number of milliseconds
int msleep(long msec);

// Get the current time
std::chrono::system_clock::time_point GetTime();

// Get a UTC ISO8601 date time string
// ie. "2012-03-02T04:07:34.0218628Z"
std::string GetDateTimeUTCISO8601(std::chrono::system_clock::time_point time);

std::string GetHomeFolder();
std::string GetConfigFolder(std::string_view sApplicationNameLower);
bool TestFileExists(const std::string& sFilePath);
bool TestFolderExists(const std::string& sFolderPath);

bool ReadFileIntoString(const std::string& sFilePath, size_t nMaxFileSizeBytes, std::string& out_contents);

}
