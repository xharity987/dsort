#ifndef MOVER_HPP
#define MOVER_HPP

#include <string>
#include <filesystem>
#include "config.hpp"

namespace fs = std::filesystem;

fs::path resolve_collision(const fs::path& dest, const std::string& collision_mode);
void move_file(const fs::path& src, const fs::path& dest, const Config& config);

#endif
