#include "connman_handler/connman_handler.hpp"
#include "ftxui/component/component.hpp"
#include "ftxui/component/menu.hpp"
#include "ftxui/dom/elements.hpp"
#include "ui/panel/panel.hpp"

namespace ui {

namespace {

void ListWifiNames(connman_handler::connman_h* connman_interface,
                   Sender<std::vector<std::wstring>> out,
                   std::function<void(void)> done) {
  std::vector<std::wstring> list;
  for (auto network : connman_interface->get_wifi_names())
    list.push_back(to_wstring(network));
  out->Send(list);
  done();
}

class WiFiImpl : public PanelBase {
 public:
  WiFiImpl(ScreenInteractive* screen) : screen_(screen) {
    wifi_toggle_ = Button(L"Toggle WiFi", [&] { ToggleWifi(); });
    scan_button = Button(&scan_label_, [&] { Scan(); });
    connect_button = Button(L"Connect", [&] { Connect(); });
    disconnect_button = Button(L"Disconnect", [&] { Disconnect(); });
    back_button = Button(L"Back", [&] { activity = ActivityMain; });
    pass_input = Input(&password, L"password");
    menu_scan = Menu(&wifi_list_, &selected);
    MenuBase::From(menu_scan)->on_enter = [&] { activity = ActivityConnect; };

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
    scan_label_ = scanning_ ? L"Scan network (Status: scanning...)"
                            : L"Scan network";
    auto wifi_status =
        (connman_interface->wifi_status() ? L"WiFi Status : Enabled"
                                          : L"WiFi Status : Disabled");
    auto current_network =
        L"Current Network: " + to_wstring(connman_interface->get_active_name());

    return vbox({
               text(wifi_status),
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
    wifi_scan_ = std::thread(ListWifiNames, connman_interface.get(),
                             wifi_list_receiver_->MakeSender(), [&] {
                               scanning_ = false;
                               screen_->PostEvent(Event::Custom);
                             });
  }

  void Connect() {
    connman_interface->data.name = to_string(wifi_list_[selected]);
    connman_interface->data.pass = to_string(password);
    connman_interface->data.type = "wifi";
    connman_interface->connect_wifi();
    activity = ActivityMain;
  }

  void Disconnect() {
    connman_interface->data.name = to_string(wifi_list_[selected]);
    connman_interface->data.pass = to_string(password);
    connman_interface->data.type = "wifi";
    connman_interface->disconnect_wifi();
    activity = ActivityMain;
  }

  void ToggleWifi() {
    if (!connman_interface->wifi_status())
      connman_interface->shell_helper("connmanctl enable wifi");
    else
      connman_interface->shell_helper("connmanctl disable wifi");
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
  std::unique_ptr<connman_handler::connman_h> connman_interface =
      std::make_unique<connman_handler::connman_h>();
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
};

}  // namespace

namespace panel {
Panel WiFi(ScreenInteractive* screen) {
  return Make<WiFiImpl>(screen);
}
}  // namespace panel

}  // namespace ui
