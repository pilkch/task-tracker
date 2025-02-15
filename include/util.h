#pragma once

#include <cstdint>

#include <chrono>
#include <string_view>

namespace util {

template <class T>
inline constexpr T clamp(const T& i, const T& lower, const T& upper) noexcept
{
  return (i < lower) ? lower : (i > upper) ? upper : i;
}

// msleep(): Sleep for the requested number of milliseconds
int msleep(long msec) noexcept;

// Get the current time
std::chrono::system_clock::time_point GetTime() noexcept;

// Get a UTC ISO8601 date time string
// ie. "2012-03-02T04:07:34.0218628Z"
std::string GetDateTimeUTCISO8601(std::chrono::system_clock::time_point time) noexcept;

std::string GetHomeFolder() noexcept;
std::string GetConfigFolder(std::string_view sApplicationNameLower) noexcept;
bool TestFileExists(const std::string& sFilePath) noexcept;
bool TestFolderExists(const std::string& sFolderPath) noexcept;

// Read a file into a string, but we put a limit on the number of bytes we will read so that we don't accidentally try to read in gigabytes of a data
bool ReadFileIntoString(const std::string& sFilePath, size_t nMaxFileSizeBytes, std::string& out_contents) noexcept;
// Write to sFilePath + ".temp", when that has successfully written we rename it to sFilePath (Prevents losing data if we crash when writing a file)
bool WriteStringToFileAtomic(const std::string& sFilePath, const std::string& contents) noexcept;

}
