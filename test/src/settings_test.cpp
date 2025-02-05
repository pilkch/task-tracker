// Task Tracker headers
#include "settings.h"

// gtest headers
#include <gtest/gtest.h>

TEST(TaskTracker, TestSettings)
{
  tasktracker::cSettings settings;
  EXPECT_TRUE(settings.LoadFromFile("test/data/configuration.json"));

  const util::cIPAddress ip = settings.GetIP();
  EXPECT_EQ(192, ip.octet0);
  EXPECT_EQ(168, ip.octet1);
  EXPECT_EQ(0, ip.octet2);
  EXPECT_EQ(3, ip.octet3);

  EXPECT_EQ(8443, settings.GetPort());

  EXPECT_EQ("https://tasktracker.mydomain.home:8443/", settings.GetExternalURL());

  const std::string https_private_key = settings.GetHTTPSPrivateKey();
  EXPECT_STREQ("./server.key", https_private_key.c_str());

  const std::string https_public_cert = settings.GetHTTPSPublicCert();
  EXPECT_STREQ("./server.crt", https_public_cert.c_str());

  EXPECT_EQ("PJYM9sAlPgoeSDu5ekFC40Q3AFJMl1uidxivonEL1NZ3DXQzzP0D8uibgnvZxbkB", settings.GetToken());
}
