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

std::string trim(const std::string& str,
                 const std::string& whitespace = " \t") {
  const auto strBegin = str.find_first_not_of(whitespace);
  if (strBegin == std::string::npos)
    return "";  // no content

  const auto strEnd = str.find_last_not_of(whitespace);
  const auto strRange = strEnd - strBegin + 1;

  return str.substr(strBegin, strRange);
}

std::string reduce(const std::string& str,
                   const std::string& fill = " ",
                   const std::string& whitespace = " \t") {
  // trim first
  auto result_local = trim(str, whitespace);

  // replace sub ranges
  auto beginSpace = result_local.find_first_of(whitespace);
  while (beginSpace != std::string::npos) {
    const auto endSpace =
        result_local.find_first_not_of(whitespace, beginSpace);
    const auto range = endSpace - beginSpace;

    result_local.replace(beginSpace, range, fill);

    const auto newStart = beginSpace + fill.length();
    beginSpace = result_local.find_first_of(whitespace, newStart);
  }

  return result_local;
}

#endif