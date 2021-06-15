#include "command_handler.hpp"

using namespace command_handler;

char cmd_h::def_gw_str[MAX_BUFFER];
char cmd_h::dns_1_str[MAX_BUFFER];
char cmd_h::dns_2_str[MAX_BUFFER];

cmd_h::cmd_h()
    : route_add(true),
      user_share_path(xdg_h::data::home()),
      user_share_config_path(user_share_path + BEAGLE_SHARE_DIR) {
  strcpy(def_gw_str, "");
  strcpy(dns_1_str, "");
  strcpy(dns_2_str, "");

  /* Load stored config */
  load_config();
}

cmd_h::~cmd_h() {}

int cmd_h::shell_h(const char* cmd, std::string check) {
  int ret = 0;
  std::array<char, 128> buffer;
  std::string result;
  std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
  if (!pipe) {
    throw std::runtime_error("popen() failed!");
  }
  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
    result += buffer.data();
  }
  if (result == check)
    ret = 1;
  else
    ret = 0;
  return ret;
}

int cmd_h::route_h() {

  /* Load Config before adding*/
  load_config();

  int sockfd;
  struct rtentry route;
  struct sockaddr_in *dst, *gw, *mask;
  int err = 0;
  auto gw_addr = inet_addr(def_gw_str);

  // create the socket
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    /*
     * Handle Error gracefully
     * Provide Logs
     */
    std::cerr << "Erro opening socket " << std::endl;
    return -1;
  }

  memset(&route, 0, sizeof(route));
  gw = (struct sockaddr_in*)&route.rt_gateway;
  gw->sin_family = AF_INET;
  gw->sin_addr.s_addr = gw_addr;

  dst = (struct sockaddr_in*)&route.rt_dst;
  dst->sin_family = AF_INET;
  dst->sin_addr.s_addr = INADDR_ANY;

  mask = (struct sockaddr_in*)&route.rt_genmask;
  mask->sin_family = AF_INET;
  mask->sin_addr.s_addr = INADDR_ANY;

  route.rt_flags = RTF_UP | RTF_GATEWAY;
  route.rt_metric = 0;

  /* 
  It is true then it adds resolv.conf and route 
  Else it deletes the route stored in beagle.conf
  */
  if (route_add) {
    std::string head_path = RESOLVCONF_PATH;

    /* It creates the resolv.conf if it does not exist */
    std::fstream resolvconf_file(head_path, std::ios::in | std::ios::app);

    /*
   ==============================================================
   Tried to disable the immutable flag after enabling below
   This enables us to temporariy write file
   Seems there is no need of it anymore
   ==============================================================
   */
    // resolvconf_file.close();
    // int attr;
    // FILE* fd = fopen(head_path.c_str(), "r");

    // ioctl(fileno(fd), FS_IOC_GETFLAGS, &attr);
    // attr |= FS_NOATIME_FL; /* Tweak returned bit mask */
    // attr = attr & (~FS_IMMUTABLE_FL);
    // ioctl(fileno(fd), FS_IOC_SETFLAGS, &attr);
    // fclose(fd);
    // resolvconf_file.open(head_path, std::ios::in | std::ios::app);
    /*
   ==============================================================
   End of disabling immutable flag
   ==============================================================
   */

    /* Stores the current line of resolv.conf*/
    std::string line;

    /* Stores the count of nameservers already present in resolv.conf*/
    size_t count = 0;
    if (resolvconf_file.is_open()) {
      if (resolvconf_file.good()) {
        while (resolvconf_file >> line) {
          /* Check if line is empty or is commented */
          if (line.empty() || line[0] == '#')
            continue;

          /* Increment count by one of "nameserver" is found */
          count += line.find(NAMESERVER_STR) != std::string::npos;
        }
        /* Since only two nameservers are allowed in resolv.conf */
        if (count < 1) {
          /* Close the file if open */
          if (resolvconf_file.is_open()) {
            resolvconf_file.close();
          }

          /* Check if it has the access */
          int write_p = access(head_path.c_str(), W_OK);

          if (write_p == 0) {
            FILE* resolv = fopen(head_path.c_str(), "w+");

            /* Get timestamp */
            std::time_t time_ = std::chrono::system_clock::to_time_t(
                std::chrono::system_clock::now());
            auto ti = std::ctime(&time_);

            /* Write comment and timestamp */
            char str[512] = "# Last Changed (by beagle-config) on ";
            strcat(str, ti);

            /* Stores nameserver strings */
            char tmp[300];
            sprintf(tmp, "\nnameserver %s\nnameserver %s\n", dns_1_str,
                    dns_2_str);
            strcat(str, tmp);

            /* Get the state of file operation */
            int state = fprintf(resolv, str);

            /* Close the file */
            fclose(resolv);
            if (state < 0) {
              printf("fail to write.\n");
            } else {
              printf("Successful %s %s\n", dns_1_str, dns_2_str);
            }
          } else {
            printf("Permission denied.\n");
          }
          std::cout << "resolv.conf edited \n";

          /*
    ==============================================================
    Tried to re enable the immutable flag after disabling above
    This prevents any programms from modifying /etc/resolv.conf
    ==============================================================
    */
          // int attr;
          // fd = fopen(head_path.c_str(), "r");

          // ioctl(fileno(fd), FS_IOC_GETFLAGS, &attr);
          // // attr |= FS_NOATIME_FL; /* Tweak returned bit mask */
          // attr = (FS_IMMUTABLE_FL);
          // ioctl(fileno(fd), FS_IOC_SETFLAGS, &attr);
          // fclose(fd);
          /*
   ==============================================================
   End of re-enabling flag
   ==============================================================
   */
        } else {
          std::cout << "Servers already exist: count - " << count << std::endl;
        }
      } else {
        std::cerr << "File not Good. Error Opening " << head_path << "\n";
      }
      if (resolvconf_file.is_open())
        resolvconf_file.close();
    } else {
      std::cerr << "Error Opening  " << head_path << "\n";
    }

    /*
    ==============================================================
    Tried Piping to resolvconf
    Did not work
    ==============================================================
    */
    // FILE* f = 0;
    // char s[256];
    // /*
    // debian@beaglebone:~$ sudo !!
    // sudo ln -s /usr/bin/resolvectl /usr/bin/resolvconf
    // debian@beaglebone:~$ resolv
    // resolvconf  resolvectl
    // */
    // snprintf(s, sizeof(s), "%s %s", "/usr/sbin/dnsmasq --port=10002 -n",
    //          "--resolv-file=");
    // if (0 != (f = popen(s, "w"))) {
    //   printf("piping to \"%s\":\n", s);
    //   if (dns_1_str && *dns_2_str) {
    //     printf("nameserver %s\n", dns_1_str);
    //     fprintf(f, "nameserver %s\n", dns_2_str);
    //   }
    //   if (dns_1_str && *dns_2_str) {
    //     printf("nameserver %s\n", dns_2_str);
    //     fprintf(f, "nameserver %s\n", dns_2_str);
    //   }
    //   if (0 <= fclose(f))
    //     return 0;
    //   perror("pclose");
    // } else {
    //   perror(s);
    // }
    /*
    ==============================================================
    Piping effort end
    ==============================================================
    */

    /*
    Remove if route exists already
    To be decided whether to have such a policy or not
    */
    // ioctl(sockfd, SIOCDELRT, &route);

    /* Add it */
    if ((err = ioctl(sockfd, SIOCADDRT, &route)) != 0) {
      /*
       * Handle Error gracefully
       * Provide Logs
       */
      std::cerr << "Error ioctl add GW: " << def_gw_str << "\n"
                << strerror(errno) << std::endl;
    }
  } else {
    if ((err = ioctl(sockfd, SIOCDELRT, &route)) != 0) {
      /*
       * Handle Error gracefully
       * Provide Logs
       */
      std::cerr << "Error ioctl del GW: " << def_gw_str << "\n"
                << strerror(errno) << std::endl;
    }
  }

  /* Close the socket */
  shutdown(sockfd, SHUT_RDWR);
  return 0;
}

int cmd_h::ics() {
  load_config();

// TODO : Verify the strings
  if (strlen(def_gw_str) < sizeof(RTGATEWAY)) {
    strcpy(def_gw_str, RTGATEWAY);
  }
  if (strlen(dns_1_str) < sizeof(DEFAULT_DNS_1)) {
    strcpy(dns_1_str, DEFAULT_DNS_1);
  }
  if (strlen(dns_2_str) < sizeof(DEFAULT_DNS_2)) {
    strcpy(dns_2_str, DEFAULT_DNS_2);
  }

  if (int err = route_h() < 0)
    return err;
  return 0;
}

void cmd_h::set_route_add_rem(bool state) {
  route_add = state;
  ics();
}

void cmd_h::store_config() {
  std::cout << "Storing:: " << user_share_config_path + BEAGLE_CONF
            << std::endl;
  config_file.open(user_share_config_path + BEAGLE_CONF);
  if (!config_file.is_open()) {
    std::cout << "Store :: Error opening config file\n";
  } else {

    /* Updating the current configuration and adding timestamp */
    std::time_t time_ =
        std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    config_file << "# Last Updated : " << std::ctime(&time_);
    config_file << "gateway"
                << "=" << def_gw_str << "\n";
    config_file << "dns1"
                << "=" << dns_1_str << "\n";
    config_file << "dns2"
                << "=" << dns_2_str << "\n";
    config_file.close();
  }
}

void cmd_h::load_config() {

  /* Checking and creating directory */
  if (!std::filesystem::is_directory(user_share_config_path)) {
    try {
      if (!std::filesystem::create_directories(user_share_config_path))
        std::cout << "Error Creating Directory\n";
    } catch (const std::exception& e) {
      std::cerr << e.what() << '\n';
    }
  }
  /* Creating config.conf */
  config_file.open(user_share_config_path + BEAGLE_CONF, std::fstream::app);
  if (!config_file.is_open()) {
    std::cout << "Not Created\n";
  } else {
    config_file.close();
    config_file.open(user_share_config_path + BEAGLE_CONF, std::fstream::in);
  }

  if (!config_file.is_open()) {
    std::cout << "load:: Error opening config file\n";
  } else {
    /* Load Config */
    /* Example From https://www.walletfox.com/course/parseconfigfile.php */

    std::string line;

    while (config_file.good() && std::getline(config_file, line)) {
      /* Removing Spaces */
      line.erase(std::remove_if(line.begin(), line.end(), ::isspace),
                 line.end());

      /* Checking if line is comment or skipping */
      if (line[0] == '#' || line.empty())
        continue;

      auto delimiterPos = line.find("=");
      auto name = line.substr(0, delimiterPos);
      auto value = line.substr(delimiterPos + 1);

      // Custom coding
      if (name == "dns1")
        strcpy(dns_1_str, value.c_str());
      else if (name == "dns2")
        strcpy(dns_2_str, value.c_str());
      else if (name == "gateway")
        strcpy(def_gw_str, value.c_str());
    }
    config_file.close();
  }
}