#pragma once

#include <string>
#include <sstream>

namespace feed {

std::string GenerateFeedID();

bool WriteFeedXML(std::ostringstream& output);

}
