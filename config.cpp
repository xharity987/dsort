#include "config.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

// Убери static, если был, чтобы функцию видели другие файлы
std::string expand_home(std::string path) {
    const char* home = getenv("HOME");
    if (home && path.starts_with("~/")) {
        return std::string(home) + path.substr(1);
    }
    return path;
}

std::string wildcard_to_regex(const std::string& pattern) {
    std::string r = "^";
    for (char c : pattern) {
        switch (c) {
            case '*': r += ".*"; break;
            case '?': r += "."; break;
            case '.': r += "\\."; break;
            default: r += c; break;
        }
    }
    r += "$";
    return r;
}

std::string clean_value(std::string s) {
    s.erase(0, s.find_first_not_of(" \t\n\r"));
    s.erase(s.find_last_not_of(" \t\n\r") + 1);
    if (s.size() >= 2 && ((s.front() == '"' && s.back() == '"') || (s.front() == '\'' && s.back() == '\''))) {
        s = s.substr(1, s.size() - 2);
    }
    return s;
}

Config load_config(const std::string& path) {
    Config config;
    std::ifstream file(expand_home(path));
    if (!file) return config;

    std::string line;
    bool reading_rules = false;
    while (std::getline(file, line)) {
        size_t comment_pos = line.find('#');
        if (comment_pos != std::string::npos) line = line.substr(0, comment_pos);
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);
        if (line.empty()) continue;

        if (line == "[rules]") { reading_rules = true; continue; }

        if (line.find('=') != std::string::npos) {
            size_t sep = line.find('=');
            std::string key = clean_value(line.substr(0, sep));
            std::string val = clean_value(line.substr(sep + 1));

            if (reading_rules) {
                Rule rule;
                rule.pattern = std::regex(wildcard_to_regex(key));
                rule.target = val;
                config.rules.push_back(rule);
            } else {
                if (key == "default") config.default_dir = val;
                else if (key == "collision") config.collision_mode = val;
                else if (key == "watch") {
                    size_t start = line.find('['), end = line.find(']');
                    if (start != std::string::npos && end != std::string::npos) {
                        std::stringstream ss(line.substr(start + 1, end - start - 1));
                        std::string item;
                        while (std::getline(ss, item, ',')) {
                            item = clean_value(item);
                            if (!item.empty()) config.watch_dirs.push_back(expand_home(item));
                        }
                    }
                }
            }
        }
    }
    return config;
}
