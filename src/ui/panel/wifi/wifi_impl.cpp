#include "connman_handler/connman_handler.hpp"
#include "ftxui/component/component.hpp"
#include "ftxui/component/menu.hpp"
#include "ftxui/dom/elements.hpp"
#include "ui/panel/panel.hpp"

using namespace ftxui;

namespace ui {

class WiFiImpl : public PanelBase {
 public:
  WiFiImpl(ScreenInteractive* screen) : screen_(screen) {
    wifi_enable_disable = Button(L"Toggle WiFi", [&] {
      if (!connman_interface->wifi_status())
        connman_interface->shell_helper("connmanctl enable wifi");
      else
        connman_interface->shell_helper("connmanctl disable wifi");
    });
    scan_button = Button(L"Scan", [&] { displayList(); });
    connect_button = Button(L"Connect", [&] {
      connect();
      depth--;
    });

    disconnect_button = Button(L"Disconnect", [&] {
      disconnect();
      depth--;
    });

    back_button = Button(L"Back", [&] { depth--; });

    pass_input = Input(&password, L"*********");

    menu_scan = Menu(&received_, &selected);
    MenuBase::From(menu_scan)->on_enter = [&] { depth++; };

    Add(Container::Tab(
        {
            Container::Vertical({
                wifi_enable_disable,
                scan_button,
                menu_scan,
            }),
            Container::Vertical({
                pass_input,
                connect_button,
                disconnect_button,
                back_button,
            }),
        },
        &depth));
  };
  ~WiFiImpl() {
    if (wifi_scan.joinable()) {
      wifi_scan.join();
    }
  }
  std::wstring Title() override { return L"WiFi"; }

  Element Render() override {
    auto scan_status = (wifi_scan.joinable() ? L"Status: --Scanned--"
                                             : L"Status: Scanning ...");

    auto wifi_status =
        (connman_interface->wifi_status() ? L"WiFi Status : Enabled"
                                          : L"WiFi Status : Disabled");
    auto current_network =
        L"Current Network: " + to_wstring(connman_interface->get_active_name());

    while (receiver_->HasPending()) {
      std::wstring str;
      receiver_->Receive(&str);
      received_.push_back(str);
    }
    Element element;
    if (depth == 0) {
      element = vbox({
                    text(wifi_status),
                    wifi_enable_disable->Render(),
                    separator(),
                    text(current_network),
                    scan_button->Render(),
                    separator(),
                    text(scan_status),
                    separator(),
                    menu_scan->Render() | yframe | yflex,
                }) |
                flex;
    } else if (depth == 1) {
      element = vbox({
                    pass_input->Render(),
                    connect_button->Render(),
                    disconnect_button->Render(),
                    back_button->Render(),
                }) |
                nothing;
    }
    return element;
  }

 private:
  void displayList() {
    if (wifi_scan.joinable()) {
      wifi_scan.join();
    } else {
      wifi_scan = std::thread([&] {
        received_.clear();
        screen_->PostEvent(Event::Custom);
        for (auto name : connman_interface->get_wifi_names()) {
          receiver_->MakeSender()->Send(to_wstring(name));
          screen_->PostEvent(Event::Custom);
        }
      });
    }
  }

  void connect() {
    connman_interface->data.name = to_string(received_[selected]);
    connman_interface->data.pass = to_string(password);
    connman_interface->data.type = "wifi";
    connman_interface->connect_wifi();
  }

  void disconnect() {
    connman_interface->data.name = to_string(received_[selected]);
    connman_interface->data.pass = to_string(password);
    connman_interface->data.type = "wifi";
    connman_interface->disconnect_wifi();
  }

  int selected = 0;
  int depth = 0;
  std::string selected_wifi;
  std::wstring password;
  std::vector<std::string> names;
  std::thread wifi_scan;
  std::unique_ptr<connman_handler::connman_h> connman_interface =
      std::make_unique<connman_handler::connman_h>();
  Component scan_button, menu_scan, pass_input, connect_button,
      disconnect_button, back_button, wifi_enable_disable;
  std::vector<std::wstring> received_;
  Receiver<std::wstring> receiver_ = MakeReceiver<std::wstring>();
  ScreenInteractive* screen_;
};

namespace panel {
Panel WiFi(ScreenInteractive* screen) {
  return Make<WiFiImpl>(screen);
}

}  // namespace panel
}  // namespace ui