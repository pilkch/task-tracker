#include <sysexits.h>

#include <iostream>
#include <sstream>

#include "settings.h"
#include "task_tracker.h"
#include "version.h"

namespace tasktracker {

void PrintUsage()
{
  std::cout<<"Usage: ./task-trackerd [OPTION]"<<std::endl;
  std::cout<<std::endl;
  std::cout<<"  -v, --version   Print the version and exit"<<std::endl;
}

void PrintVersion()
{
  std::cout<<"task-trackerd version "<<version<<std::endl;
}

}

int main(int argc, char* argv[])
{
  if (argc == 2) {
    const std::string argument(argv[1]);
    if ((argument == "-v") || (argument == "--version")) {
      tasktracker::PrintVersion();
      return EXIT_SUCCESS;
    } else {
      // Unknown argument, print the usage and exit
      tasktracker::PrintUsage();
      return EX_USAGE;
    }
  } else if (argc != 1) {
    // Incorrect number of arguments, print the usage and exit
    tasktracker::PrintUsage();
    return EX_USAGE;
  }


  // Parse the configuration file
  tasktracker::cSettings settings;
  if (!settings.LoadFromFile("./configuration/configuration.json")) {
    std::cerr<<"Error parsing configuration/configuration.json"<<std::endl;
    return EXIT_FAILURE;
  }

  const bool result = tasktracker::RunServer(settings);

  return (result ? EXIT_SUCCESS : EXIT_FAILURE);
}
