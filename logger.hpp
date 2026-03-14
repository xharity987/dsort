#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>
#include <string>
#include <syslog.h>

enum LogLevel {
    LOG_QUIET,
    LOG_NORMAL,
    LOG_VERBOSE
};

class Logger {
private:
    bool is_daemon;
    LogLevel level;
    
public:
    Logger(bool daemon_mode = false, LogLevel log_level = LOG_NORMAL) 
        : is_daemon(daemon_mode), level(log_level) {
        if (is_daemon) {
            openlog("dsort", LOG_PID | LOG_NDELAY, LOG_USER);
        }
    }
    
    ~Logger() {
        if (is_daemon) {
            closelog();
        }
    }
    
    void info(const std::string& message) {
        if (level == LOG_QUIET) return;
        if (is_daemon) {
            syslog(LOG_INFO, "%s", message.c_str());
        } else {
            std::cout << message << std::endl;
        }
    }
    
    void verbose(const std::string& message) {
        if (level != LOG_VERBOSE) return;
        if (is_daemon) {
            syslog(LOG_DEBUG, "%s", message.c_str());
        } else {
            std::cout << "[VERBOSE] " << message << std::endl;
        }
    }
    
    void error(const std::string& message) {
        if (is_daemon) {
            syslog(LOG_ERR, "%s", message.c_str());
        } else {
            std::cerr << "ERROR: " << message << std::endl;
        }
    }
    
    void warning(const std::string& message) {
        if (is_daemon) {
            syslog(LOG_WARNING, "%s", message.c_str());
        } else {
            std::cerr << "WARNING: " << message << std::endl;
        }
    }
};

#endif
