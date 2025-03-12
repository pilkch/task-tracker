// Task Tracker headers
#include "settings.h"

// gtest headers
#include <gtest/gtest.h>

TEST(TaskTracker, TestSettings)
{
  tasktracker::cSettings settings;
  EXPECT_TRUE(settings.LoadFromFile("test/configuration/configuration.json"));

  EXPECT_TRUE(settings.GetRunningInContainer());

  const util::cIPAddress ip = settings.GetIP();
  EXPECT_EQ(192, ip.octet0);
  EXPECT_EQ(168, ip.octet1);
  EXPECT_EQ(0, ip.octet2);
  EXPECT_EQ(3, ip.octet3);

  EXPECT_EQ(8443, settings.GetPort());

  EXPECT_EQ("https://tasktracker.mydomain.home:8443/", settings.GetExternalURL());
  EXPECT_STREQ("./test/configuration/unit_test_server.key", settings.GetHTTPSPrivateKey().c_str());
  EXPECT_STREQ("./test/configuration/unit_test_server.crt", settings.GetHTTPSPublicCert().c_str());
  EXPECT_EQ("u5ekFC43AFJMl1uidPJYM9P0D8uibgnvZxbk0QsAlPgoeSDxivonEL1NZ3DXQzzB", settings.GetToken());

  EXPECT_STREQ("https://gitlab.mydomain.home:2443/", settings.GetGitlabURL().c_str());
  EXPECT_STREQ("glfgi-ijcxzvZXCJIO58FD348s", settings.GetGitlabAPIToken().c_str());
  EXPECT_STREQ("./test/configuration/gitlab_server.crt", settings.GetGitlabHTTPSPublicCert().c_str());
}
