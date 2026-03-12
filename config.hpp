#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include <vector>
#include <regex>

struct Rule {
    std::regex pattern;
    std::string target;
};

struct Config {
    std::vector<std::string> watch_dirs;
    std::string default_dir = "Other";
    std::string collision_mode = "rename";
    std::string log_level = "normal";
    std::vector<Rule> rules;
    std::vector<std::regex> ignore_patterns;
};

// Добавь эту строку:
std::string expand_home(std::string path);

Config load_config(const std::string& path);

#endif
