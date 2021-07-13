#include <arpa/inet.h>
#include <net/route.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <filesystem>
#include <fstream>
#include "ftxui/component/component.hpp"
#include "ftxui/dom/elements.hpp"
#include "ui/panel/panel.hpp"
#include "xdg_utils.hpp"

#define RTGATEWAY "192.168.6.1"
#define RESOLVCONF_PATH "/etc/resolv.conf"
#define NAMESERVER_STR "nameserver"
#define DEFAULT_DNS_1 "8.8.8.8"
#define DEFAULT_DNS_2 "8.8.4.4"

#define BEAGLE_SHARE_DIR "/beagle-config"
#define BEAGLE_CONF "/beagle.conf"

#define MAX_BUFFER 16

using namespace ftxui;

namespace ui {

class ICSImpl : public PanelBase {
 public:
  ICSImpl()
      : route_add(true),
        user_share_path(xdg_utils::data::home()),
        user_share_config_path(user_share_path + BEAGLE_SHARE_DIR) {
    /* Load stored config */
    load_config();
    Add(Container::Vertical({
        Container::Horizontal({
            Button(L"ICS On",
                   [&] {
                     route_add = true;
                     route_h();
                     store_config();
                   }),
            Button(L"ICS Off",
                   [&] {
                     route_add = false;
                     route_h();
                     store_config();
                   }),
        }),
        Container::Horizontal({
            Input(&def_gw_str, L" Gateway "),
            Input(&dns_1_str, L" DNS 1 "),
            Input(&dns_2_str, L" DNS 2 "),
        }),
    }));
  }
  ~ICSImpl() = default;
  std::wstring Title() override {
    return L"Internet Sharing and Client Configuration";
  }

 private:
  bool route_add;

  /*Storing XDG Paths */
  const std::string user_share_path;
  const std::string user_share_config_path;

  /* Storing User config in the below file */
  std::fstream config_file;

  /*
  Internet Connection Sharing Main Function 
  * Edits resolv.conf
  * Adds route
  * Deletes route
  TODO:Verify the userinput
  */
  int route_h() {
    int sockfd;
    struct rtentry route;
    struct sockaddr_in *dst, *gw, *mask;
    int err = 0;
    auto gw_addr = inet_addr(to_string(def_gw_str).c_str());

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
              sprintf(tmp, "nameserver %s\nnameserver %s\n",
                      to_string(dns_1_str).c_str(),
                      to_string(dns_2_str).c_str());
              strcat(str, tmp);

              /* Get the state of file operation */
              int state = fprintf(resolv, "%s", str);

              /* Close the file */
              fclose(resolv);
              if (state < 0) {
                printf("fail to write.\n");
              } else {
                printf("Successful %s %s\n", to_string(dns_1_str).c_str(),
                       to_string(dns_2_str).c_str());
              }
            } else {
              printf("Permission denied.\n");
            }
            std::cout << "resolv.conf edited \n";
          } else {
            std::cout << "Servers already exist: count - " << count
                      << std::endl;
          }
        } else {
          std::cerr << "File not Good. Error Opening " << head_path << "\n";
        }
        if (resolvconf_file.is_open())
          resolvconf_file.close();
      } else {
        std::cerr << "Error Opening  " << head_path << "\n";
      }

      /* Add it */
      if ((err = ioctl(sockfd, SIOCADDRT, &route)) != 0) {
        /*
         * Handle Error gracefully
         * Provide Logs
         */
        std::cerr << "Error ioctl add GW: " << to_string(def_gw_str) << "\n"
                  << strerror(errno) << std::endl;
      }
    } else {
      if ((err = ioctl(sockfd, SIOCDELRT, &route)) != 0) {
        /*
         * Handle Error gracefully
         * Provide Logs
         */
        std::cerr << "Error ioctl del GW: " << to_string(def_gw_str) << "\n"
                  << strerror(errno) << std::endl;
      }
    }

    /* Close the socket */
    shutdown(sockfd, SHUT_RDWR);
    return 0;
  }

  /* Load Config */
  void load_config() {
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
          dns_1_str = to_wstring(value);
        else if (name == "dns2")
          dns_2_str = to_wstring(value);
        else if (name == "gateway")
          def_gw_str = to_wstring(value);
      }
      config_file.close();
    }
  }

  /* Store Config */
  void store_config() {
    std::cout << "Storing:: " << user_share_config_path + BEAGLE_CONF
              << std::endl;
    config_file.open(user_share_config_path + BEAGLE_CONF);
    if (!config_file.is_open()) {
      std::cout << "Store :: Error opening config file\n";
    } else {
      /* Updating the current configuration and adding timestamp */
      std::time_t time_ = std::chrono::system_clock::to_time_t(
          std::chrono::system_clock::now());

      config_file << "# Last Updated : " << std::ctime(&time_);
      config_file << "gateway"
                  << "=" << to_string(def_gw_str) << "\n";
      config_file << "dns1"
                  << "=" << to_string(dns_1_str) << "\n";
      config_file << "dns2"
                  << "=" << to_string(dns_2_str) << "\n";
      config_file.close();
    }
  }

  std::wstring def_gw_str;
  std::wstring dns_1_str;
  std::wstring dns_2_str;
};

namespace panel {
Panel ICS() {
  return Make<ICSImpl>();
}

}  // namespace panel
}  // namespace ui
