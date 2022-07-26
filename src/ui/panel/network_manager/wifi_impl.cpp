#include <errno.h>
#include <linux/if.h>
#include <linux/wireless.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <algorithm>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>
#include "ftxui/component/component.hpp"
#include "ftxui/dom/elements.hpp"
#include "ui/panel/panel.hpp"
#include "utils.hpp"
#include "network_manager.hpp"

namespace ui {

namespace {

constexpr const char CONNMAN_SERVICE_F_PATH[]{"/var/lib/connman/"};

/* Definition of possible strings in the .config files */
constexpr const char SERVICE_KEY_TYPE[]{"Type"};
constexpr const char SERVICE_KEY_NAME[]{"Name"};
constexpr const char SERVICE_KEY_PASSPHRASE[]{"Passphrase"};

struct connman_data {
  /* wifi | ethernet */
  std::string type;

  /* SSID */
  std::string name;

  /* Password */
  std::string pass;
};

NetworkManager nm{};
Wifi_Objects wifi_objects = nm.get_wifi_objects();

class WiFiImpl : public PanelBase {
 public:
  WiFiImpl(ScreenInteractive* screen) : screen_(screen) {
    // TODO:Is there a better method than this?
    // if (!std::filesystem::exists("/sys/class/net/wlan0")) {
    //   wifiCompatiblity = false;
    // }

    if (wifiCompatiblity) {
      wifi_toggle_ = Button("Toggle WiFi", [&] { ToggleWifi(); });
      scan_button = Button(&scan_label_, [&] { Scan(); });
      connect_button = Button("Connect", [&] { Connect(); });
      disconnect_button = Button("Disconnect", [&] { Disconnect(); });
      forget_button = Button("Forget", [&] { Forget(); });
      back_button = Button("Back", [&] { activity = ActivityMain; });
      back_button1 = Button("Back", [&] { activity = ActivityMain; });
      back_button2 = Button("Back", [&] { activity = ActivityMain; });
      pass_input = Input(&password, "password");
      option.on_enter = [&] { activity = ActivityConnect; };
      option1.on_enter = [&] { activity = ActivityDisconnect; };
      option2.on_enter = [&] { activity = ActivityForget; };
      menu_scan = Menu(&wifi_list_, &selected, &option);
      menu_active = Menu(&wifi_active_, &selected1, &option1);
      menu_save = Menu(&wifi_save_, &selected2, &option2);

      Add(Container::Tab(
          {
              Container::Vertical({
                  Container::Horizontal({
                      wifi_toggle_,
                      scan_button,
                  }),
                  menu_active,
                  menu_scan,
                  menu_save,
              }),
              Container::Vertical({
                  pass_input,
                  Container::Horizontal({
                      connect_button,
                      back_button,
                  }),
              }),
              Container::Horizontal({
                  disconnect_button,
                  back_button1,
              }),
              Container::Horizontal({
                  forget_button,
                  back_button2,
              }),
          },
          &activity)
          );
      // Populate the wifi network in the background.
      // Scan();
      RefreshNetworkList();
    }
  };
  ~WiFiImpl() {
    if (wifiCompatiblity)
      if (wifi_scan_.joinable()) {
        wifi_scan_.join();
      }
  }
  std::string Title() override { return "WiFi"; }

  Element Render() override {
    if (wifiCompatiblity) {
      while (wifi_list_receiver_->HasPending())
        wifi_list_receiver_->Receive(&wifi_list_);

      if (activity == ActivityMain)
        return RenderMain();

      if (activity == ActivityConnect)
        return RenderConnect();

      if (activity == ActivityDisconnect)
        return RenderDisconnect();

      if (activity == ActivityForget)
        return RenderForget();

      return text("Not implemented");

    } else {
      return text("Feature not supported");
    }
  }

  Element RenderMain() {
    scan_label_ =
        scanning_ ? "Scan network (Status: scanning...)" : "Scan network";
    auto wifi_status_ =
        (WifiStatus() ? "WiFi Status : Enabled" : "WiFi Status : Disabled");
    // auto current_network = "Current Network: " + ActiveWifiName();

    return vbox({
               text(wifi_status_),
              //  text(current_network),
               hbox({
                   wifi_toggle_->Render(),
                   scan_button->Render(),
               }),
               window(text("Active Connection"), menu_active->Render() | yframe | size(HEIGHT, EQUAL, 1)),
               window(text("Avaliable Network"), menu_scan->Render() | yframe | size(HEIGHT, LESS_THAN, 10)),
               window(text("Saved Network"), menu_save->Render() | yframe | size(HEIGHT, LESS_THAN, 10)),
           }) |
           flex;
  }

  Element RenderConnect() {
    return vbox({
               vbox({
                   hbox({text("Network : "), text(wifi_list_[selected])}),
                   hbox({text("Password: "), pass_input->Render()}),
               }),
               hbox({
                   connect_button->Render(),
                   back_button->Render(),
               }),
           }) |
           nothing;
  }

  Element RenderDisconnect() {
    return vbox({
              vbox({
                   hbox({text("WiFi : "), text(wifi_active_[selected1])}),
              }),
              hbox({
                  disconnect_button->Render(),
                  back_button1->Render(),
              }),
           }) |
           nothing;
  }

  Element RenderForget() {
    return vbox({
              vbox({
                   hbox({text("Network : "), text(wifi_save_[selected2])}),
              }),
              hbox({
                  forget_button->Render(),
                  back_button2->Render(),
              }),
           }) |
           nothing;
  }

 private:
  void Scan() {
    if (wifi_scan_.joinable())
      wifi_scan_.join();

    // scanning_ = true;
    // wifi_scan_ = std::thread([=] {
    //   ListWifiNames(wifi_list_receiver_->MakeSender(), [=] {
    //     scanning_ = false;
    //     screen_->PostEvent(Event::Custom);
    //   });
    // });

    scanning_ = true;
    activity = ActivityMain;
    RefreshNetworkList();
    wifi_scan_ = std::thread([=] {
      scanning_ = false;
      screen_->PostEvent(Event::Custom);
    });
  }

  void Connect() {
    nm.saved_wireless(selected, password);
  }

  void Forget() {
    nm.forget_network(selected2);

    activity = ActivityMain;

    RefreshNetworkList();
  }

  void Disconnect() {
    nm.disconnect_active_WiFi();
    
    activity = ActivityMain;

    RefreshNetworkList();
  }

  void ToggleWifi() {
    if (!WifiStatus()) {
      nm.enable_NetworkManager();
    }
    else {
      nm.disable_NetworkManager();
    }

    RefreshNetworkList();
  }

  void ListWifiNames(Sender<std::vector<std::string>> out,
                     std::function<void(void)> done) {
    std::vector<std::string> list;
    RefreshNetworkList();
    for (auto network : service_names)
      list.push_back(network.first);
    out->Send(list);
    done();
  }

  int StoreConnmanFile(const char* path) {
    std::fstream connman_config(
        path, std::fstream::in | std::fstream::out | std::fstream::trunc);
    if (!connman_config.is_open()) {
      return -1;
    } else {
      std::time_t time_ = std::chrono::system_clock::to_time_t(
          std::chrono::system_clock::now());
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

  void EmptyConnmanConfig(const char* path) {
    std::fstream connman_config(
        path, std::fstream::in | std::fstream::out | std::fstream::trunc);
    if (!connman_config.is_open()) {
    } else {
      std::time_t time_ = std::chrono::system_clock::to_time_t(
          std::chrono::system_clock::now());
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

  /*
    Get wifi SSIDs and unique names
    1. Enables wifi
    2. Scans
    3. Parses the output
  */
  void RefreshNetworkList() {
    nm.refresh_wifi();
    wifi_list_.clear();
    wifi_active_.clear();
    wifi_save_.clear();

    wifi_objects = nm.get_wifi_objects();
    
    uint i;
    if (!wifi_objects.new_access_points.empty())
      for (i = 0; i < wifi_objects.new_access_points.size(); i++)
      {
          wifi_list_.push_back(wifi_objects.new_access_points[i].ssid);
      }

    if (!wifi_objects.active_access_point.aceesspoint_dir.empty())
      wifi_active_.push_back(wifi_objects.active_access_point.ssid);

    if (!wifi_objects.saved_access_points.empty())
      for (i = 0; i < wifi_objects.saved_access_points.size(); i++)
      {
          wifi_save_.push_back(wifi_objects.saved_access_points[i].ssid);
      }
  }

  std::string ActiveWifiName() {
    int sockfd;
    char active_wlan_id[IW_ESSID_MAX_SIZE + 1];

    struct iwreq wlan_ioctl_req;
    memset(&wlan_ioctl_req, 0, sizeof(struct iwreq));
    wlan_ioctl_req.u.essid.length = IW_ESSID_MAX_SIZE + 1;

    sprintf(wlan_ioctl_req.ifr_name, "wlan0");

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
      return active_name = std::string("Error: ") + *strerror(errno);
    }

    wlan_ioctl_req.u.essid.pointer = active_wlan_id;
    if (ioctl(sockfd, SIOCGIWESSID, &wlan_ioctl_req) == -1) {
      return active_name = std::string("Error: ") + *strerror(errno);
    }
    close(sockfd);
    active_name = (char*)wlan_ioctl_req.u.essid.pointer;
    if (active_name.length())
      return active_name;
    return active_name = "None";
  }

  bool WifiStatus() {
    if (wifi_objects.network_enable)
      return 1;
    else 
      return 0;
  }

  int selected = 0, selected1 = 0, selected2 = 0;
  enum Activity : int {
    ActivityMain,
    ActivityConnect,
    ActivityDisconnect,
    ActivityForget,
  };
  int activity = ActivityMain;
  std::string selected_wifi;
  std::string password;
  std::vector<std::string> names;
  std::thread wifi_scan_;
  connman_data data;
  Component scan_button;
  Component menu_scan;
  Component menu_active;
  Component menu_save;
  MenuOption option;
  MenuOption option1;
  MenuOption option2;
  Component pass_input;
  Component connect_button;
  Component disconnect_button;
  Component forget_button;
  Component back_button;
  Component back_button1;
  Component back_button2;
  Component wifi_toggle_;
  std::vector<std::string> wifi_list_;
  std::vector<std::string> wifi_active_;
  std::vector<std::string> wifi_save_;
  bool scanning_ = false;
  bool wifiCompatiblity = true;
  std::string scan_label_;
  Receiver<decltype(wifi_list_)> wifi_list_receiver_ =
      MakeReceiver<decltype(wifi_list_)>();
  ScreenInteractive* screen_;
  /* Command Result */
  std::string result;

  /* Current Wifi name */
  std::string service_name;
  std::string active_name;

  /*
   * List of normal - unique names from connmanctl
   * John ----- wifi_<macaddress>_<hash>_managed_psk
   */
  std::unordered_map<std::string, std::string> service_names;

  /* Service name and its key-value */
  std::unordered_map<std::string, std::unordered_map<std::string, std::string>>
      lookup_table;
};

}  // namespace

namespace panel {
Panel NetMan(ScreenInteractive* screen) {
  return Make<WiFiImpl>(screen);
}
}  // namespace panel

}  // namespace ui
