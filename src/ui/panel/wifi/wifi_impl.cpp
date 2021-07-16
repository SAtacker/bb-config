#include "connman_handler/connman_handler.hpp"
#include "ftxui/component/component.hpp"
#include "ftxui/dom/elements.hpp"
#include "ui/panel/panel.hpp"
#include "utils.hpp"

namespace ui {

namespace {

struct connman_data {
  /* wifi | ethernet */
  std::string type;

  /* SSID */
  std::string name;

  /* Password */
  std::string pass;
};

class WiFiImpl : public PanelBase {
 public:
  WiFiImpl(ScreenInteractive* screen) : screen_(screen) {
    wifi_toggle_ = Button(L"Toggle WiFi", [&] { ToggleWifi(); });
    scan_button = Button(&scan_label_, [&] { Scan(); });
    connect_button = Button(L"Connect", [&] { Connect(); });
    disconnect_button = Button(L"Disconnect", [&] { Disconnect(); });
    back_button = Button(L"Back", [&] { activity = ActivityMain; });
    pass_input = Input(&password, L"password");
    MenuOption option;
    option.on_enter = [&] { activity = ActivityConnect; };
    menu_scan = Menu(&wifi_list_, &selected, &option);

    Add(Container::Tab(
        {
            Container::Vertical({
                Container::Horizontal({
                    wifi_toggle_,
                    scan_button,
                }),
                menu_scan,
            }),
            Container::Vertical({
                pass_input,
                Container::Horizontal({
                    connect_button,
                    disconnect_button,
                    back_button,
                }),
            }),
        },
        &activity));

    // Populate the wifi network in the background.
    Scan();
  };
  ~WiFiImpl() {
    if (wifi_scan_.joinable()) {
      wifi_scan_.join();
    }
  }
  std::wstring Title() override { return L"WiFi"; }

  Element Render() override {
    while (wifi_list_receiver_->HasPending())
      wifi_list_receiver_->Receive(&wifi_list_);

    if (activity == ActivityMain)
      return RenderMain();

    if (activity == ActivityConnect)
      return RenderConnect();

    return text(L"Not implemented");
  }

  Element RenderMain() {
    scan_label_ =
        scanning_ ? L"Scan network (Status: scanning...)" : L"Scan network";
    auto wifi_status_ =
        (wifi_status() ? L"WiFi Status : Enabled" : L"WiFi Status : Disabled");
    auto current_network = L"Current Network: " + to_wstring(ActiveWifiName());

    return vbox({
               text(wifi_status_),
               text(current_network),
               hbox({
                   wifi_toggle_->Render(),
                   scan_button->Render(),
               }),
               window(text(L"Network"), menu_scan->Render() | yframe),
           }) |
           flex;
  }

  Element RenderConnect() {
    return vbox({
               vbox({
                   hbox({text(L"Network : "), text(wifi_list_[selected])}),
                   hbox({text(L"Password: "), pass_input->Render()}),
               }),
               hbox({
                   connect_button->Render(),
                   disconnect_button->Render(),
                   back_button->Render(),
               }),
           }) |
           nothing;
  }

 private:
  void Scan() {
    if (wifi_scan_.joinable())
      wifi_scan_.join();

    scanning_ = true;
    wifi_scan_ = std::thread([&] {
      ListWifiNames(wifi_list_receiver_->MakeSender(), [&] {
        scanning_ = false;
        screen_->PostEvent(Event::Custom);
      });
    });
  }

  void Connect() {
    data.name = to_string(wifi_list_[selected]);
    data.pass = to_string(password);
    data.type = "wifi";
    std::string file_path;
    RefreshNetworkList();
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
      return;
    }
    std::cout << "Calling store: " << file_path << std::endl;
    StoreConnmanFile(file_path.c_str());
    activity = ActivityMain;
  }

  void Disconnect() {
    data.name = to_string(wifi_list_[selected]);
    data.pass = to_string(password);
    data.type = "wifi";
    std::string file_path;
    RefreshNetworkList();
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
    EmptyConnmanConfig(file_path.c_str());
    activity = ActivityMain;
  }

  void ToggleWifi() {
    if (!wifi_status())
      shell_helper("connmanctl enable wifi");
    else
      shell_helper("connmanctl disable wifi");
  }

  void ListWifiNames(Sender<std::vector<std::wstring>> out,
                     std::function<void(void)> done) {
    std::vector<std::wstring> list;
    RefreshNetworkList();
    for (auto network : service_names)
      list.push_back(to_wstring(network.first));
    out->Send(list);
    done();
  }

  int StoreConnmanFile(const char* path) {
    std::cout << "Storing in " << path << std::endl;
    std::fstream connman_config(
        path, std::fstream::in | std::fstream::out | std::fstream::trunc);
    if (!connman_config.is_open()) {
      std::cerr << "Error opening connman config " << path << std::endl;
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
    std::cout << "Emptying " << path << std::endl;
    std::fstream connman_config(
        path, std::fstream::in | std::fstream::out | std::fstream::trunc);
    if (!connman_config.is_open()) {
      std::cerr << "Emptying: Error opening connman config " << path
                << std::endl;
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

  std::string ActiveWifiName() {
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

  bool wifi_status() {
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

  std::string trim(const std::string& str, const std::string& whitespace = " \t") {
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

  int selected = 0;
  enum Activity : int {
    ActivityMain,
    ActivityConnect,
  };
  int activity = ActivityMain;
  std::string selected_wifi;
  std::wstring password;
  std::vector<std::string> names;
  std::thread wifi_scan_;
  connman_data data;
  Component scan_button;
  Component menu_scan;
  Component pass_input;
  Component connect_button;
  Component disconnect_button;
  Component back_button;
  Component wifi_toggle_;
  std::vector<std::wstring> wifi_list_;
  bool scanning_ = false;
  std::wstring scan_label_;
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
Panel WiFi(ScreenInteractive* screen) {
  return Make<WiFiImpl>(screen);
}
}  // namespace panel

}  // namespace ui
