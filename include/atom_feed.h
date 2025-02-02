#pragma once

#include <string>
#include <sstream>

#include "feed_data.h"
#include "random.h"

namespace feed {

std::string GenerateFeedID(util::cPseudoRandomNumberGenerator& rng);

bool WriteFeedXML(const tasktracker::cFeedData& feed_data, std::ostringstream& output);

}
