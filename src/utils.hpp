#ifndef UTILS_HPP
#define UTILS_HPP

#include "process.hpp"

void shell_helper(const char* cmd, std::string* result) {
  *result = "";

  procxx::process shell{"sh"};
  procxx::process::limits_t limits;
  limits.cpu_time(1);

  shell.add_argument("-c");
  shell.add_argument(cmd);

  shell.limit(limits);
  shell.exec();

  std::string line;
  while (std::getline(shell.output(), line))
    *result = *result + line + "\n";
}

void shell_helper(const char* cmd) {
  procxx::process shell{"sh"};
  procxx::process::limits_t limits;
  limits.cpu_time(2);

  shell.add_argument("-c");
  shell.add_argument(cmd);

  shell.limit(limits);
  shell.exec();
}

#endif