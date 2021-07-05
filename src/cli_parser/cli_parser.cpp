#include "cli_parser.hpp"

using namespace cli_parser;

static struct option long_options[] = {
    /* Gateway is the routing gateway */
    {"gateway", required_argument, 0, 0},

    /* Dns nameservers */
    {"dns1", required_argument, 0, 0},
    {"dns2", required_argument, 0, 0},

    /* Does not edit resolv.conf */
    {"no-resolv", no_argument, 0, 0},

    /* No GUI (More to be added ) */
    {"no-gui", no_argument, 0, 'n'},
    {0, 0, 0, 0},
};

args_parser::args_parser() {}

args_parser::~args_parser() {}

void args_parser::parse(int& argc, char* const argv[]) {
  if (argc > 1) {
    while ((c = getopt_long(argc, argv, "i:::", long_options, &option_index)) !=
           -1) {
      switch (c) {
        // If it is long option, 0 would be returned.
        case 0: {
          if (long_options[option_index].name == "gateway") {
            /* Stores gateway in static variable and stores the config*/
            func_c_i_vec.at(0)(optarg, 0);
          } else if (long_options[option_index].name == "dns1") {
            /* Stores gateway in static variable and stores the config*/
            func_c_i_vec.at(0)(optarg, 1);
          } else if (long_options[option_index].name == "dns2") {
            /* Stores gateway in static variable and stores the config*/
            func_c_i_vec.at(0)(optarg, 2);
          } else if (long_options[option_index].name == "no-resolv") {
            /* Does not edit resolv.conf */
            func_c_i_vec.at(0)(optarg, 4);
          }
          break;
        }
        case 'i': {
          /* Adds routes and edits resolv.conf */
          func_c_i_vec.at(0)(optarg, 3);
          break;
        }
        case '?': {
          std::cout << "Got unknown option." << std::endl;
          func_vec.at(0)();
          break;
        }
        default: {
          func_vec.at(0)();
          std::cout << "Got unknown parse returns: " << c << std::endl;
        }
      }
    }
    for (int i = optind; i < argc; i++) {
      std::cout << "Non-option argument: " << std::endl;
      std::cout << argv[i] << std::endl;
    }
  } else {
    func_vec.at(0)();
  }
}

void args_parser::add_function(std::function<void(void)> func) {
  func_vec.push_back(func);
}

void args_parser::add_function(std::function<void(char*, int)> func) {
  func_c_i_vec.push_back(func);
}