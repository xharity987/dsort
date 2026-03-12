#include "cli.hpp"
#include <iostream>
#include <getopt.h>
#include <cstdlib>

void print_usage() {
    std::cout << "Usage: dsort [OPTIONS]\n"
              << "Options:\n"
              << "  -c, --config PATH    Path to config file (default: ~/.config/dsort/config.toml)\n"
              << "  -n, --dry-run        Dry run (do not move files)\n"
              << "  -s, --scan           Scan once and exit\n"
              << "  -d, --daemon         Run in daemon mode\n"
              << "  -h, --help           Show this help message\n";
}

void parse_args(int argc, char** argv, std::string& config_path, bool& dry_run, bool& scan, bool& daemon) {
    static struct option long_options[] = {
        {"config", required_argument, nullptr, 'c'},
        {"dry-run", no_argument, nullptr, 'n'},
        {"scan", no_argument, nullptr, 's'},
        {"daemon", no_argument, nullptr, 'd'},
        {"help", no_argument, nullptr, 'h'},
        {nullptr, 0, nullptr, 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "c:nsdh", long_options, nullptr)) != -1) {
        switch (opt) {
            case 'c': config_path = optarg; break;
            case 'n': dry_run = true; break;
            case 's': scan = true; break;
            case 'd': daemon = true; break;
            case 'h': print_usage(); exit(0);
            default: print_usage(); exit(1);
        }
    }
}
