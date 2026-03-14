#ifndef WATCHER_HPP
#define WATCHER_HPP

#include <functional>
#include <string>
#include <atomic>
#include "config.hpp"

using FileEventHandler = std::function<void(const std::string&)>;

void start_watching(const Config& config, FileEventHandler handler, std::atomic<bool>& running);

#endif
