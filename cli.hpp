#ifndef CLI_HPP
#define CLI_HPP

#include <string>

void print_usage();
void parse_args(int argc, char** argv, std::string& config_path, bool& dry_run, bool& scan, bool& daemon);

#endif
