#include "connman_handler.hpp"

using namespace connman_handler;

std::vector<std::string> const connman_h::service_possible_keys = {
    SERVICE_KEY_TYPE,
    SERVICE_KEY_NAME,
    SERVICE_KEY_SSID,
    SERVICE_KEY_PASSPHRASE,
};

connman_h::connman_h() : val_change(false) {}

connman_h::~connman_h() {}

void connman_h::parse_file(const char* path) {
  std::fstream connman_config(path, std::ios::in | std::ios::out);
  if (!connman_config.is_open()) {
    std::cerr << "Error opening connman config" << std::endl;
  } else {
    bool is_block_started = false;
    std::string line;
    while (connman_config >> line) {
      if (line.empty()) {
        is_block_started = false;
        continue;
      } else if (line[0] == '#') {
        continue;
      } else if (line[0] == '[') {
        line.erase(line.begin());
        line.erase(line.end());
        auto _ = line.find("_");
        service_name = line.substr(_);
        is_block_started = true;
        continue;
      }
      /* Removing Spaces */
      line.erase(std::remove_if(line.begin(), line.end(), ::isspace),
                 line.end());
      if (is_block_started) {
        auto delimiterPos = line.find("=");
        auto name = line.substr(0, delimiterPos);
        auto value = line.substr(delimiterPos + 1);

        for (auto service_token : service_possible_keys) {
          if (service_token.compare(name) == 0) {
            auto lkup = lookup_table[service_name].find(name);
            if (lkup != lookup_table[service_name].end()) {
              if (lkup->second != value && val_change)
                lookup_table[service_name][name] = value;
            }
          }
        }
      }
    }
  }
}

int connman_h::store_file(const char* path) {
  std::cout << "Storing in " << path << std::endl;
  std::fstream connman_config(
      path, std::fstream::in | std::fstream::out | std::fstream::trunc);
  if (!connman_config.is_open()) {
    std::cerr << "Error opening connman config " << path << std::endl;
    return -1;
  } else {
    std::time_t time_ =
        std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    auto ti = std::ctime(&time_);
    connman_config << "# Edited by beagle config: " << ti << "\n";
    for (auto sname : lookup_table) {
      if (sname.second[SERVICE_KEY_NAME] == data.name) {
        connman_config << "[service_" << sname.first << "]"
                       << "\n";
        for (auto name_value : sname.second) {
          connman_config << name_value.first << " = " << name_value.second
                         << "\n";
        }
        connman_config << "\n";
      }
    }
  }
  return 0;
}

void connman_h::empty_file(const char* path) {
  std::cout << "Emptying " << path << std::endl;
  std::fstream connman_config(
      path, std::fstream::in | std::fstream::out | std::fstream::trunc);
  if (!connman_config.is_open()) {
    std::cerr << "Emptying: Error opening connman config " << path << std::endl;
  } else {
    std::time_t time_ =
        std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    auto ti = std::ctime(&time_);
    connman_config << "# Edited by beagle config: " << ti << "\n";
    for (auto sname : lookup_table) {
      if (sname.second[SERVICE_KEY_NAME] == data.name) {
        connman_config << "#[service_" << sname.first << "]"
                       << "\n";
        for (auto name_value : sname.second) {
          connman_config << "#" << name_value.first << " = "
                         << name_value.second << "\n";
        }
        connman_config << "\n";
      }
    }
  }
}

void connman_h::shell_helper(const char* cmd) {
  result = "";

  procxx::process shell{"sh"};
  procxx::process::limits_t limits;
  limits.cpu_time(1);

  shell.add_argument("-c");
  shell.add_argument(cmd);

  shell.limit(limits);
  shell.exec();

  std::string line;
  while (std::getline(shell.output(), line))
    result = result + line + "\n";

  /* ----------------------------------------------------------------------
  // This is not thread safe
  std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
  if (!pipe) {
    throw std::runtime_error("popen() failed!");
  }
  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
    result += buffer.data();
  }
   ---------------------------------------------------------------------- */

  // Below code uses STDOUT and STDERR which are same as that of FTXUI and
  // break the UI
  /* ----------------------------------------------------------------------
  pid_t pid = 0;
  int pipefd[2];
  FILE* output;

  pipe(pipefd);  // create a pipe
  pid = fork();  // span a child process
  if (pid == 0) {
    // Child. Let's redirect its standard output to our pipe and replace
    process
    // with tail
    close(pipefd[0]);
    dup2(pipefd[1], STDOUT_FILENO);
    dup2(pipefd[1], STDERR_FILENO);
    setvbuf(stdout, NULL, _IOLBF, 1000);
    execl("/bin/sh", "sh", "-c", cmd, (char*)NULL);
  }

  // Only parent gets here. Listen to what the tail says
  close(pipefd[1]);
  output = fdopen(pipefd[0], "r");

  while (fgets(buffer.data(), buffer.size(), output) != nullptr) {
    result += buffer.data();
  }
  fclose(output);
   ---------------------------------------------------------------------- */
}

void connman_h::get_service_names() {
  if (!wifi_status()) {
    /* Enable wifi */
    shell_helper("connmanctl enable wifi");
  }

  /* Scan wifi */
  shell_helper("connmanctl scan wifi");

  /* Get mac address formatted into continuous string */
  shell_helper(
      "cat /sys/class/net/wlan0/address | sed  -r "
      "'s/^([^:]{2}):([^:]{2}):([^:]{2}):([^:]{2}):([^:]{2}):([^:]{2})$/"
      "\\1\\2\\3\\4\\5\\6/'");

  /* Get macaddress from trailing spaces */
  auto mac_address = result.substr(0, result.length() - 2);

  /* Get connman services */
  shell_helper("connmanctl services | sed -e 's/[ \t]*//'");

  /* Temporary string to store wifi_macaddress*/
  auto temp_start = "wifi_" + mac_address;

  while (true) {
    /* If length is 0 break */
    if (result.length() == 0) {
      break;
    }

    /* Position of newline character */
    size_t newline_pos = 1;
    if (result.find("\n") != std::string::npos) {
      newline_pos = result.find("\n");

      /* Get current line from trimmed sub sequence */
      auto current_line = reduce(result.substr(0, newline_pos));

      /* Position of wifi_<macaddress> */
      auto pos = current_line.find(temp_start);

      /* Default check for std::string::npos */
      if (pos != std::string::npos) {
        /* pos==0 means that it is hidden and has no name*/
        if (pos != 0) {
          auto name = reduce(current_line.substr(0, pos));
          auto act_pos = name.find("*A");
          if (act_pos != std::string::npos) {
            name.erase(act_pos, act_pos + name.find_first_of(" ") + 1);
            active_name = name;
          }
          auto unique_name = current_line.substr(pos);
          service_names[name] = unique_name;
        } else {
          service_names["hidden - " + current_line.substr(0, 10)] =
              current_line;
        }
      } else {
        std::cerr << "Not finding unique_name" << std::endl;
      }
    }
    result.erase(0, newline_pos + 1);
  }
}

void connman_h::display_service_names() {
  for (auto sn : service_names) {
    std::cout << "Name :: " << sn.first << std::endl;
    std::cout << "Service:: " << sn.second << std::endl;
    std::cout << std::endl;
  }
}

std::string connman_h::trim(const std::string& str,
                            const std::string& whitespace) {
  const auto strBegin = str.find_first_not_of(whitespace);
  if (strBegin == std::string::npos)
    return "";  // no content

  const auto strEnd = str.find_last_not_of(whitespace);
  const auto strRange = strEnd - strBegin + 1;

  return str.substr(strBegin, strRange);
}

std::string connman_h::reduce(const std::string& str,
                              const std::string& fill,
                              const std::string& whitespace) {
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

int connman_h::connect_wifi() {
  std::string file_path;
  get_service_names();
  for (auto n : service_names) {
    if (n.first == data.name) {
      file_path = CONNMAN_SERVICE_F_PATH "/" + n.second + ".config";
      lookup_table[n.second][SERVICE_KEY_TYPE] = data.type;
      lookup_table[n.second][SERVICE_KEY_NAME] = data.name;
      lookup_table[n.second][SERVICE_KEY_PASSPHRASE] = data.pass;
      break;
    }
  }
  if (file_path.empty()) {
    std::cerr << "Network Not found :- " << data.name << std::endl;
    return -1;
  }
  std::cout << "Calling store: " << file_path << std::endl;
  return store_file(file_path.c_str());
}

void connman_h::disconnect_wifi() {
  std::string file_path;
  get_service_names();
  for (auto n : service_names) {
    if (n.first == data.name) {
      file_path = CONNMAN_SERVICE_F_PATH "/" + n.second + ".config";
      lookup_table[n.second][SERVICE_KEY_TYPE] = data.type;
      lookup_table[n.second][SERVICE_KEY_NAME] = data.name;
      lookup_table[n.second][SERVICE_KEY_PASSPHRASE] = data.pass;
      break;
    }
  }
  std::cout << "Calling empty: " << file_path << std::endl;
  empty_file(file_path.c_str());
}

std::vector<std::string> connman_h::get_wifi_names() {
  get_service_names();
  std::vector<std::string> vec;
  for (auto n : service_names) {
    vec.push_back(n.first);
  }
  return vec;
}

bool connman_h::wifi_status() {
  int skfd = socket(AF_INET, SOCK_DGRAM, 0);

  if (skfd < 0) {
    std::cerr << "cannot open socket" << std::endl;
    return 0;
  }

  const char* ifname = "wlan0";
  struct ifreq req;

  strncpy(req.ifr_ifrn.ifrn_name, ifname, IFNAMSIZ);

  int err = ioctl(skfd, SIOCGIFFLAGS, &req);
  if (err) {
    perror("SIOCGIFFLAGS");
    return err;
  } else {
    int flags = req.ifr_ifru.ifru_flags;
    return (flags & IFF_UP) ? true : false;
  }
  return 0;
}

std::string connman_h::get_active_name() {
  int sockfd;
  char id[IW_ESSID_MAX_SIZE + 1];

  struct iwreq wreq;
  memset(&wreq, 0, sizeof(struct iwreq));
  wreq.u.essid.length = IW_ESSID_MAX_SIZE + 1;

  sprintf(wreq.ifr_name, "wlan0");

  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
    return active_name = std::string("Error: ") + *strerror(errno);
  }

  wreq.u.essid.pointer = id;
  if (ioctl(sockfd, SIOCGIWESSID, &wreq) == -1) {
    return active_name = std::string("Error: ") + *strerror(errno);
  }
  close(sockfd);
  active_name = (char*)wreq.u.essid.pointer;
  if (active_name.length())
    return active_name;
  return active_name = "None";
}
