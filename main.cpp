#include <iostream>
#include <string>
#include <filesystem>
#include <vector>
#include <atomic>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <fstream>
#include "config.hpp"
#include "mover.hpp"
#include "watcher.hpp"
#include "rules.hpp"
#include "cli.hpp"
#include "signals.hpp"
#include <sys/inotify.h>

extern std::atomic<bool> stop_daemon;
namespace fs = std::filesystem;

void daemonize() {
    pid_t pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);
    
    setsid();
    
    pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);
    
    chdir("/");
    
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    
    open("/dev/null", O_RDWR);
    dup(0);
    dup(0);
    
    umask(022);
}

void write_pid_file() {
    std::ofstream pid_file("/tmp/dsort.pid");
    if (pid_file.is_open()) {
        pid_file << getpid() << std::endl;
    }
}

void handle_file_event(const Config& config, bool dry_run, const std::string& filepath) {
    fs::path src_path(filepath);
    if (!fs::exists(src_path)) return;

    std::string filename = src_path.filename().string();
    if (is_ignored(config, filename)) return;

    std::string target = find_target(config, filename);
    if (!target.empty()) {
        fs::path target_path = expand_home(target);
        fs::path dest_dir;

        if (target_path.is_absolute()) {
            dest_dir = target_path;
        } else {
            dest_dir = src_path.parent_path() / target_path;
        }

        fs::path final_dest = dest_dir / filename;

        if (dry_run) {
            std::cout << "[DRY RUN] " << filename << " -> " << final_dest << std::endl;
        } else {
            move_file(src_path, final_dest, config);
        }
    }
}

int main(int argc, char* argv[]) {
    std::string config_path = "~/.config/dsort/config.toml";
    bool dry_run = false;
    bool scan_only = false;
    bool daemon_mode = false;

    parse_args(argc, argv, config_path, dry_run, scan_only, daemon_mode);
    Config config = load_config(config_path);

    // Демонизация
    if (daemon_mode && !scan_only) {
        daemonize();
        write_pid_file();
    }

    setup_signal_handlers();

    if (scan_only) {
        for (const auto& dir : config.watch_dirs) {
            if (!fs::exists(dir)) continue;
            for (const auto& entry : fs::directory_iterator(dir)) {
                if (entry.is_regular_file()) {
                    handle_file_event(config, dry_run, entry.path().string());
                }
            }
        }
        if (!daemon_mode) return 0;
    }

    // Бесконечный цикл с inotify
    std::cout << "Starting dsort daemon..." << std::endl;
    
    int fd = inotify_init();
    if (fd < 0) {
        std::cerr << "Failed to init inotify" << std::endl;
        return 1;
    }

    for (const auto& dir : config.watch_dirs) {
        if (fs::exists(dir)) {
            inotify_add_watch(fd, dir.c_str(), IN_CLOSE_WRITE | IN_MOVED_TO);
        }
    }

    char buffer[4096];
    while (!stop_daemon) {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        
        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        
        if (select(fd + 1, &fds, NULL, NULL, &tv) > 0) {
            int len = read(fd, buffer, sizeof(buffer));
            if (len > 0) {
                for (char* ptr = buffer; ptr < buffer + len; ) {
                    auto* event = (inotify_event*)ptr;
                    if (event->len > 0 && !(event->mask & IN_ISDIR)) {
                        for (const auto& dir : config.watch_dirs) {
                            std::string full_path = dir + "/" + event->name;
                            if (fs::exists(full_path)) {
                                handle_file_event(config, dry_run, full_path);
                                break;
                            }
                        }
                    }
                    ptr += sizeof(inotify_event) + event->len;
                }
            }
        }
    }

    close(fd);
    if (daemon_mode) remove("/tmp/dsort.pid");
    return 0;
}
