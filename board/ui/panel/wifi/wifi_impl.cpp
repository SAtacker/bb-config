#include "connman_handler/connman_handler.hpp"
#include "ftxui/component/component.hpp"
#include "ftxui/dom/elements.hpp"
#include "ui/panel/panel.hpp"

using namespace ftxui;

namespace ui {

class WiFiImpl : public PanelBase {
 private:
 public:
  WiFiImpl(ScreenInteractive* screen) : screen_(screen) {
    scan_button = Button(
        L"Scan", [this] { displayList(); }, false);
    Add(scan_button);
  };
  ~WiFiImpl() {
    if (wifi_scan.joinable()) {
      wifi_scan.join();
    }
  }
  std::wstring Title() override { return L"WiFi"; }

  Element Render() override {
    auto scan_status =
        (wifi_scan.joinable() ? L"--Scanned--" : L"Scanning ...");

    while (receiver_->HasPending()) {
      std::wstring str;
      receiver_->Receive(&str);
      received_.push_back(str);
    }
    Elements received_list;
    for (const auto& item : received_)
      received_list.push_back(text(item));
    received_list.push_back(text(scan_status) | ftxui::select);

    return vbox({
               scan_button->Render(),
               separator(),
               vbox(std::move(received_list)) | yframe | yflex,
           }) |
           flex;
  }

 private:
  void displayList() {
    if (wifi_scan.joinable()) {
      wifi_scan.join();
    } else {
      receiver_->MakeSender()->Send(L"Calling...");
      screen_->PostEvent(Event::Custom);
      wifi_scan = std::thread([&] {
        for (auto name : connman_interface->get_wifi_names()) {
          receiver_->MakeSender()->Send(to_wstring(name));
          screen_->PostEvent(Event::Custom);
        }
      });
    }
  }

  int selected = 0;
  std::string selected_wifi;
  std::vector<std::string> names;
  std::thread wifi_scan;
  std::unique_ptr<connman_handler::connman_h> connman_interface;
  Component scan_button;
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
