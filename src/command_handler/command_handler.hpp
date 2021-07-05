#ifndef COMMAND_HANDLER_HPP
#define COMMAND_HANDLER_HPP

#include <arpa/inet.h>
#include <errno.h>
#include <linux/fs.h>
#include <net/route.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <algorithm>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include "../xdg_app_handler/xdg_handler.hpp"

#define RTGATEWAY "192.168.6.1"
#define RESOLVCONF_PATH "/etc/resolv.conf"
#define NAMESERVER_STR "nameserver"
#define DEFAULT_DNS_1 "8.8.8.8"
#define DEFAULT_DNS_2 "8.8.4.4"

#define BEAGLE_SHARE_DIR "/beagle-config"
#define BEAGLE_CONF "/beagle.conf"

#define MAX_BUFFER 16

namespace command_handler {
class cmd_h {
 private:
  bool route_add;

  /*Storing XDG Paths */
  const std::string user_share_path;
  const std::string user_share_config_path;

  /* Storing User config in the below file */
  std::fstream config_file;

 public:
  cmd_h();
  ~cmd_h();

  /* Helper function for shell operations */
  int shell_h(const char*, std::string check = "");

  /* Internet Connection Sharing Main Function */
  int ics();

  /* Driver Function for Internet Connection Sharing*/
  void set_route_add_rem(bool);

  /* Routing Helper Function*/
  int route_h();

  /* Load Config */
  void load_config();

  /* Store Config */
  void store_config();

  static char def_gw_str[MAX_BUFFER];
  static char dns_1_str[MAX_BUFFER];
  static char dns_2_str[MAX_BUFFER];
};

}  // namespace command_handler

#endif  // End of include guard: COMMAND_HANDLER_HPP
