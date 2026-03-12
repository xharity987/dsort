#ifndef WATCHER_HPP
#define WATCHER_HPP

#include "config.hpp"
#include <string>
#include <functional>
#include <atomic>

using FileEventHandler = std::function<void(const std::string&)>;

void start_watching(const Config& config, FileEventHandler handler, std::atomic<bool>& watcher_running);

#endif
