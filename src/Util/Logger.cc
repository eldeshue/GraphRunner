
#include "Logger.h"

#include <memory>

// Static member initialization
std::unique_ptr<GraphRunner::Util::Logger> GraphRunner::Util::Logger::instance =
    nullptr;
