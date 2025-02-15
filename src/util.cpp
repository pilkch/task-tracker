#include <cerrno>
#include <ctime>

#include <chrono>
#include <filesystem>
#include <format>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include <pwd.h>
#include <sys/stat.h>
#include <unistd.h>

namespace util {

// From: https://stackoverflow.com/a/1157217/1074390
int msleep(long msec) noexcept
{
  struct timespec ts;
  int res;

  if (msec < 0) {
    errno = EINVAL;
    return -1;
  }

  ts.tv_sec = msec / 1000;
  ts.tv_nsec = (msec % 1000) * 1000000;

  do {
    res = nanosleep(&ts, &ts);
  } while (res && errno == EINTR);

  return res;
}

std::chrono::system_clock::time_point GetTime() noexcept
{
  return std::chrono::system_clock::now();
}

// Get a UTC ISO8601 date time string
// ie. "2012-03-02T04:07:34.0218628Z"
std::string GetDateTimeUTCISO8601(std::chrono::system_clock::time_point time) noexcept
{
  const std::string raw = std::format("{:%FT%TZ}", time);

  // HACK: The default formatting string returns something like "2012-03-02T04:07:34.0218628Z", but we only want 3 decimal places for the milliseconds,
  // so we truncate them here and add the Z again, taking care to make sure we always truncate the Z on the end, even if there were only a few millisecond decimal places
  return raw.substr(0, std::min<size_t>(std::max<size_t>(raw.length(), 1) - 1, 23)) + "Z";
}

std::string GetHomeFolder() noexcept
{
  const char* szHomeFolder = getenv("HOME");
  if (szHomeFolder != nullptr) return szHomeFolder;

  struct passwd* pPasswd = getpwuid(getuid());
  if (pPasswd != nullptr) return pPasswd->pw_dir;

  return "";
}

std::string GetConfigFolder(std::string_view sApplicationNameLower) noexcept
{
  const std::string sHomeFolder = GetHomeFolder();
  if (sHomeFolder.empty()) return "";

  return sHomeFolder + "/.config/" + std::string(sApplicationNameLower);
}

bool TestFileExists(const std::string& sFilePath) noexcept
{
  struct stat s;
  return (stat(sFilePath.c_str(), &s) >= 0);
}

bool TestFolderExists(const std::string& sFolderPath) noexcept
{
  return TestFileExists(sFolderPath);
}

size_t GetFileSizeBytes(const std::string& sFilePath) noexcept
{
  struct stat s;
  if (stat(sFilePath.c_str(), &s) < 0) return 0;

  return s.st_size;
}

bool ReadFileIntoString(const std::string& sFilePath, size_t nMaxFileSizeBytes, std::string& contents) noexcept
{
  if (!TestFileExists(sFilePath)) {
    std::cerr<<"File \""<<sFilePath<<"\" not found"<<std::endl;
    return false;
  }

  const size_t nFileSizeBytes = GetFileSizeBytes(sFilePath);
  if (nFileSizeBytes == 0) {
    std::cerr<<"Empty file \""<<sFilePath<<"\""<<std::endl;
    return false;
  } else if (nFileSizeBytes > nMaxFileSizeBytes) {
    std::cerr<<"File \""<<sFilePath<<"\" is too large"<<std::endl;
    return false;
  }

  std::ifstream f(sFilePath);

  contents.reserve(nFileSizeBytes);

  contents.assign(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());

  return true;
}

bool WriteStringToFileAtomic(const std::string& sFilePath, const std::string& contents) noexcept
{
  const std::string sFilePathTemp = sFilePath + ".temp";

  {
    // Write to the temp file
    std::ofstream f(sFilePathTemp);
    f<<contents;
  }

  // Remove the old file if it exists
  std::error_code ec;
  std::filesystem::remove(sFilePath, ec);

  // Rename the temp file to overwrite the real file
  std::filesystem::rename(sFilePathTemp, sFilePath, ec);

  return true;
}

}
