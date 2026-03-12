#ifndef SIGNALS_HPP
#define SIGNALS_HPP

#include <atomic>

extern std::atomic<bool> running;
extern std::atomic<bool> reload_config;

void setup_signal_handlers();

#endif
