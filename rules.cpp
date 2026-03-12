#include "rules.hpp"
#include <regex>

bool is_ignored(const Config& config, const std::string& filename) {
    for (const auto& pattern : config.ignore_patterns) {
        if (std::regex_match(filename, pattern)) {
            return true;
        }
    }
    return false;
}

std::string find_target(const Config& config, const std::string& filename) {
    for (const auto& rule : config.rules) {
        if (std::regex_match(filename, rule.pattern)) {
            return rule.target;
        }
    }
    return config.default_dir;
}
