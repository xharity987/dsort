#include "watcher.hpp"
#include <sys/inotify.h>
#include <sys/select.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <filesystem>

void start_watching(const Config& config, FileEventHandler handler, std::atomic<bool>& watcher_running) {
    int fd = inotify_init1(IN_NONBLOCK);
    if (fd < 0) {
        std::cerr << "Failed to initialize inotify" << std::endl;
        return;
    }

    int watched_count = 0;
    for (const auto& dir : config.watch_dirs) {
        if (!std::filesystem::exists(dir)) {
            if (config.log_level == "verbose") {
                std::cout << "Directory does not exist: " << dir << std::endl;
            }
            continue;
        }

        int wd = inotify_add_watch(fd, dir.c_str(), IN_CLOSE_WRITE | IN_MOVED_TO);
        if (wd < 0) {
            std::cerr << "Failed to watch directory: " << dir << std::endl;
        } else {
            watched_count++;
            if (config.log_level == "verbose") {
                std::cout << "Watching: " << dir << std::endl;
            }
        }
    }

    if (watched_count == 0) {
        std::cerr << "No directories to watch!" << std::endl;
        close(fd);
        return;
    }

    char buffer[4096] __attribute__((aligned(8)));

    // Главный цикл - работает пока watcher_running == true
    while (watcher_running) {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(fd, &fds);

        struct timeval timeout;
        timeout.tv_sec = 1; // Проверяем флаг каждую секунду
        timeout.tv_usec = 0;

        int ret = select(fd + 1, &fds, nullptr, nullptr, &timeout);

        if (ret < 0) {
            if (errno == EINTR) continue; // Прервано сигналом - пробуем снова
            std::cerr << "select error: " << strerror(errno) << std::endl;
            break;
        }

        if (ret > 0) {
            int len = read(fd, buffer, sizeof(buffer));
            if (len < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) continue;
                std::cerr << "read error: " << strerror(errno) << std::endl;
                break;
            }

            for (char* ptr = buffer; ptr < buffer + len; ) {
                auto* event = reinterpret_cast<inotify_event*>(ptr);

                if (!(event->mask & IN_ISDIR) && event->len > 0) {
                    // Ищем правильную директорию для этого события
                    for (const auto& dir : config.watch_dirs) {
                        std::string full_path = dir + "/" + event->name;
                        if (std::filesystem::exists(full_path)) {
                            handler(full_path);
                            break;
                        }
                    }
                }
                ptr += sizeof(inotify_event) + event->len;
            }
        }
    }

    // Закрываем fd только при выходе из цикла
    close(fd);
    if (config.log_level == "verbose") {
        std::cout << "Watcher stopped" << std::endl;
    }
}
