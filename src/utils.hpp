#ifndef UTILS_HPP
#define UTILS_HPP

#include "process.hpp"

void shell_helper(const char* cmd, std::string* result);
void shell_helper(const char* cmd);
void shell_helper_no_limit(const char* cmd);

std::string trim(const std::string& str, const std::string& whitespace = " \t");

std::string reduce(const std::string& str,
                   const std::string& fill = " ",
                   const std::string& whitespace = " \t");

#endif