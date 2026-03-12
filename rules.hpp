#ifndef RULES_HPP
#define RULES_HPP

#include "config.hpp"
#include <string>

bool is_ignored(const Config& config, const std::string& filename);
std::string find_target(const Config& config, const std::string& filename);

#endif
