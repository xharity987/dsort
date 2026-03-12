#include "mover.hpp"
#include <iostream>

fs::path resolve_collision(const fs::path& dest, const std::string& collision_mode) {
    if (!fs::exists(dest)) return dest;

    if (collision_mode == "skip") return ""; // Возвращаем пустой путь для пропуска
    if (collision_mode == "overwrite") return dest;
    
    int i = 1;
    fs::path new_dest = dest.parent_path() / (dest.stem().string() + "(" + std::to_string(i) + ")" + dest.extension().string());

    while (fs::exists(new_dest)) {
        i++;
        new_dest = dest.parent_path() / (dest.stem().string() + "(" + std::to_string(i) + ")" + dest.extension().string());
    }

    return new_dest;
}

void move_file(const fs::path& src, const fs::path& dest, const Config& config) {
    // Создаем целевую директорию, если её нет
    if (!fs::exists(dest.parent_path())) {
        fs::create_directories(dest.parent_path());
    }

    fs::path resolved_dest = resolve_collision(dest, config.collision_mode);

    if (resolved_dest.empty()) {
        if (config.log_level != "quiet") {
            std::cout << "Skipped (collision): " << src.filename() << std::endl;
        }
        return;
    }

    try {
        fs::rename(src, resolved_dest);
        if (config.log_level != "quiet") {
            std::cout << "Moved: " << src.filename() << " -> " << resolved_dest.parent_path().filename() << "/" << resolved_dest.filename() << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Failed to move " << src << " to " << resolved_dest << ": " << e.what() << std::endl;
    }
}
