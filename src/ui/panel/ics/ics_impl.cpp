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

using namespace ftxui;

namespace ui {

namespace {

std::string UserShareConfigPath() {
  return xdg_utils::data::home() + "/beagle-config";
}

std::string BeagleConfigPath() {
  return UserShareConfigPath() + "/beagle.conf";
}

int CountNameserver() {
  size_t count = 0;
  std::ifstream file("/etc/resolv.conf");

  std::string line;
  while (std::getline(file, line)) {
    // Skip empty or commented lines.
    if (line.empty() || line[0] == '#')
      continue;

    bool namespace_found = line.find("nameserver") != std::string::npos;
    if (namespace_found)
      count++;
  }
  return count;
}

class ICSImpl : public PanelBase {
 public:
  ICSImpl() {
    LoadConfig();
    Add(Container::Vertical({
        Container::Horizontal({
            Button(L"ICS On",
                   [&] {
                     route_add_ = true;
                     route_h();
                     StoreConfig();
                   }),
            Button(L"ICS Off",
                   [&] {
                     route_add_ = false;
                     route_h();
                     StoreConfig();
                   }),
        }),
        Container::Horizontal({
            Input(&gateway_str, L" Gateway "),
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
  bool route_add_ = true;

  // Internet Connection Sharing Main Function
  // - Edits resolv.conf
  // - Adds route
  // - Deletes route
  // - TODO:Verify the userinput
  int route_h() {
    // Create a socket.
    int socket_file_descriptor = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_file_descriptor < 0) {
      std::cerr << "Error opening socket " << std::endl;
      return -1;
    }

    rtentry route;
    memset(&route, 0, sizeof(route));

    std::string gateway_string = to_string(gateway_str);
    auto gw_addr = inet_addr(gateway_string.c_str());

    sockaddr_in* rt_gateway = reinterpret_cast<sockaddr_in*>(&route.rt_gateway);
    rt_gateway->sin_family = AF_INET;
    rt_gateway->sin_addr.s_addr = gw_addr;

    sockaddr_in* rt_dst = reinterpret_cast<sockaddr_in*>(&route.rt_dst);
    rt_dst->sin_family = AF_INET;
    rt_dst->sin_addr.s_addr = INADDR_ANY;

    sockaddr_in* rt_genmask = reinterpret_cast<sockaddr_in*>(&route.rt_genmask);
    rt_genmask->sin_family = AF_INET;
    rt_genmask->sin_addr.s_addr = INADDR_ANY;

    route.rt_flags = RTF_UP | RTF_GATEWAY;
    route.rt_metric = 0;

    // It is true then it adds resolv.conf and route else it deletes the route
    // stored in beagle.conf
    if (!route_add_) {
      int error =  ioctl(socket_file_descriptor, SIOCDELRT, &route);
      if (error != 0) {
        std::cerr << "Error ioctl del rt_gateway: " << to_string(gateway_str) << "\n"
                  << strerror(errno) << std::endl;
      }
      shutdown(socket_file_descriptor, SHUT_RDWR);
      return -1;
    }

    int nameserver_count = CountNameserver();
    if (nameserver_count >= 2) {
      std::cerr << "Servers already exist: nameserver_count = "
                << nameserver_count << std::endl;

      shutdown(socket_file_descriptor, SHUT_RDWR);
      return -1;
    }

    /* Check if it has the access */
    std::ofstream file("/etc/resolv.conf", std::ofstream::app);

    if (!file.good()) {
      std::cerr << "Permission denied" << std::endl;
      shutdown(socket_file_descriptor, SHUT_RDWR);
      return -1;
    }


    std::time_t time_ =
        std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    auto ti = std::ctime(&time_);
    file << "# Last changed by beagle-config on: " << ti << std::endl;

    /* Stores nameserver strings */
    file << "nameserver " << to_string(dns_1_str) << std::endl;
    file << "nameserver " << to_string(dns_2_str) << std::endl;

    std::cerr << "Successful " << to_string(dns_1_str) << " "
              << to_string(dns_2_str) << std::endl;

    /* Add it */
    int error = ioctl(socket_file_descriptor, SIOCADDRT, &route);
    if (error != 0) {
      std::cerr << "Error ioctl add rt_gateway: " << to_string(gateway_str) << "\n"
                << strerror(errno) << std::endl;
      shutdown(socket_file_descriptor, SHUT_RDWR);
      return -1;
    }

    /* Close the socket */
    shutdown(socket_file_descriptor, SHUT_RDWR);
    return 0;
  }

  // Load Config
  void LoadConfig() {
    // Create the config directory if it doesn't exist yet.
    if (!std::filesystem::is_directory(UserShareConfigPath())) {
      if (!std::filesystem::create_directories(UserShareConfigPath())) {
        std::cerr << "Error Creating Directory" << std::endl;
        return;
      }
    }

    // Creating config.conf */
    std::ifstream config_file(BeagleConfigPath());
    if (!config_file) {
      std::cerr << "load:: Error opening config file" << std::endl;
      return;
    }

    /* Load Config */
    /* Example From https://www.walletfox.com/course/parseconfigfile.php */
    std::string line;
    while (std::getline(config_file, line)) {
      // Removing Spaces */
      line.erase(std::remove_if(line.begin(), line.end(), ::isspace),
                 line.end());

      // Skip comments and empty lines.
      if (line[0] == '#' || line.empty())
        continue;

      // We expect name=value entries.
      auto delimiterPos = line.find("=");
      if (delimiterPos == std::string::npos)  // Delimiter not found
        continue;

      auto name = line.substr(0, delimiterPos);
      auto value = line.substr(delimiterPos + 1);

      if (name == "dns1") {
        dns_1_str = to_wstring(value);
        continue;
      }

      if (name == "dns2") {
        dns_2_str = to_wstring(value);
        continue;
      }

      if (name == "gateway") {
        gateway_str = to_wstring(value);
        continue;
      }
    }
  }

  void StoreConfig() {
    std::cerr << "Storing:: " << BeagleConfigPath() << std::endl;
    std::ofstream config_file(BeagleConfigPath());
    if (!config_file) {
      std::cerr << "Store :: Error opening config file" << std::endl;
      return;
    }

    // Updating the current configuration and adding timestamp
    std::time_t time_ =
        std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    config_file << "# Last Updated : " << std::ctime(&time_);
    config_file << "gateway=" << to_string(gateway_str) << std::endl;
    config_file << "dns1=" << to_string(dns_1_str) << std::endl;
    config_file << "dns2=" << to_string(dns_2_str) << std::endl;
  }

  std::wstring gateway_str;
  std::wstring dns_1_str;
  std::wstring dns_2_str;
};

}  // namespace
namespace panel {
Panel ICS() {
  return Make<ICSImpl>();
}

}  // namespace panel
}  // namespace ui
