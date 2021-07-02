#ifndef CLI_PARSER_HPP
#define CLI_PARSER_HPP

#include <getopt.h>
#include <functional>
#include <iostream>
#include <vector>

namespace cli_parser {

class args_parser {
 private:
  /* Stores getopt return value */
  int c;

  /* Stores option index */
  int option_index;

  /* Vectors to get functions from main() */
  std::vector<std::function<void(void)>> func_vec;
  std::vector<std::function<void(char*, int)>> func_c_i_vec;

 public:
  args_parser();

  /* Parses */
  void parse(int& argc, char* const argv[]);

  /* Adds void functions */
  void add_function(std::function<void(void)>);

  /* Adds void(char*,int) functions */
  void add_function(std::function<void(char*, int)>);
  ~args_parser();
};

}  // namespace cli_parser

#endif  // End of Include Guard : CLI_PARSER_HPP