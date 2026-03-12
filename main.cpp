#include <iostream>
#include <string>
#include <filesystem>
#include <vector>
#include <atomic>
#include "config.hpp"
#include "mover.hpp"
#include "watcher.hpp"
#include "rules.hpp"
#include "cli.hpp"
#include "signals.hpp"
extern std::atomic<bool> stop_daemon;
namespace fs = std::filesystem;

void handle_file_event(const Config& config, bool dry_run, const std::string& filepath) {
    fs::path src_path(filepath);
    if (!fs::exists(src_path)) return;
    
    std::string filename = src_path.filename().string();
    if (is_ignored(config, filename)) return;
    
    std::string target = find_target(config, filename);
    if (!target.empty()) {
        // 1. Разворачиваем ~ в /home/user
        fs::path target_path = expand_home(target);
        fs::path dest_dir;

        // 2. КЛЮЧЕВОЙ МОМЕНТ: Проверяем, абсолютный ли путь
        if (target_path.is_absolute()) {
            dest_dir = target_path;
        } else {
            // Относительный путь — кладем внутрь текущей папки
            dest_dir = src_path.parent_path() / target_path;
        }
        
        fs::path final_dest = dest_dir / filename;

        if (dry_run) {
            std::cout << "[DRY RUN] " << filename << " -> " << final_dest << std::endl;
        } else {
            // Печатаем для отладки, куда реально переносим
            std::cout << "Moving: " << filename << " -> " << final_dest << std::endl;
            move_file(src_path, final_dest, config);
        }
    }
}

void scan_existing_files(const Config& config, bool dry_run) {
    if (config.log_level != "quiet") std::cout << "Scanning existing files..." << std::endl;
    for (const auto& dir : config.watch_dirs) {
        if (!fs::exists(dir)) continue;
        for (const auto& entry : fs::directory_iterator(dir)) {
            if (entry.is_regular_file()) {
                handle_file_event(config, dry_run, entry.path().string());
            }
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
    
    setup_signal_handlers(); 

    if (scan_only) {
        scan_existing_files(config, dry_run);
        if (!daemon_mode) return 0;
    }

    if (config.log_level != "quiet") std::cout << "Starting dsort daemon..." << std::endl;
    
    start_watching(config, [&](const std::string& path) {
        handle_file_event(config, dry_run, path);
    }, stop_daemon);

    return 0;
}
