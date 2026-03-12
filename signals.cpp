#include "signals.hpp"
#include <signal.h>
#include <iostream>

std::atomic<bool> running(true);
std::atomic<bool> reload_config(false);
std::atomic<bool> stop_daemon(false); // Определение здесь!

void handle_signal(int sig) {
    if (sig == SIGINT || sig == SIGTERM) {
        running = false;
    }
    if (sig == SIGHUP) {
        reload_config = true;
    }
}

void setup_signal_handlers() {
    struct sigaction sa;
    sa.sa_handler = handle_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGINT, &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr);
    sigaction(SIGHUP, &sa, nullptr);
}
