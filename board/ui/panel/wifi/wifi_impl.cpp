#include "connman_handler/connman_handler.hpp"
#include "ftxui/component/component.hpp"
#include "ftxui/dom/elements.hpp"
#include "ui/panel/panel.hpp"

using namespace ftxui;

namespace ui {

void Connect(std::string name, std::string pass, Sender<std::wstring> out) {
  std::unique_ptr<connman_handler::connman_h> connman_interface;
  connman_interface->data.name = name;
  connman_interface->data.pass = pass;
  connman_interface->data.type = "wifi";
  connman_interface->connect_wifi();
  auto connected_name = connman_interface->get_active_name();
  if (connected_name != "None") {
    out->Send(to_wstring(connected_name));
  }
  out->Send(L"Not Connected");
}

class WiFiImpl : public PanelBase {
 private:
 public:
  WiFiImpl() {}
  ~WiFiImpl() = default;
  std::wstring Title() override { return L"WiFi"; }

  Element Render() override {
    Elements e;
    for (auto n : connman_interface->get_wifi_names()) {
      e.push_back(text(to_wstring(n)));
      e.push_back(separator());
    }

    return vbox(e) | flex;
  }

 private:
  int depth = 0;
  std::string selected_wifi;
  std::vector<std::wstring> received_;
  int selected = 0;
  Receiver<std::wstring> receiver_ = MakeReceiver<std::wstring>();
  std::unique_ptr<connman_handler::connman_h> connman_interface;
};

namespace panel {
Panel WiFi() {
  return Make<WiFiImpl>();
}

}  // namespace panel
}  // namespace ui
