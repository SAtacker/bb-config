#include <filesystem>
#include <fstream>
#include <unordered_map>
#include "ftxui/dom/elements.hpp"
#include "ui/panel/panel.hpp"

using namespace ftxui;

namespace ui {

// A basic implementation of PanelBase with no internal logic.
class PRUImpl : public PanelBase {
 public:
  PRUImpl() {
    updateStatus();

    for (const auto& name : pruList) {
      auto onStr = name + L" On";
      auto offStr = name + L" Off";
      pruButtons.push_back(std::make_pair<Component, Component>(
          Button(onStr, [&] { changePruState(name, true); }),
          Button(offStr, [&] { changePruState(name, false); })));
    }
    Components buttons;
    for (auto button : pruButtons) {
      buttons.push_back(Container::Horizontal({
          button.first,
          button.second,
      }));
    }
    Add(Container::Vertical(std::move(buttons)));
  }
  ~PRUImpl() = default;
  std::wstring Title() override { return L"PRU enable/disable"; }

 private:
  void changePruState(std::wstring pru, bool state_) {
    auto statePath = to_string(pruNamePath[pru] + L"/state");
    auto state = state_ ? "start" : "stop";
    std::ofstream(statePath) << state;
  }

  void updateStatus() {
    pruList.clear();
    for (const auto& rproc :
         std::filesystem::directory_iterator("/sys/class/remoteproc/")) {
      std::string state, name, fw;
      std::ifstream(std::string(rproc.path()) + "/state") >> state;
      std::ifstream(std::string(rproc.path()) + "/name") >> name;
      std::ifstream(std::string(rproc.path()) + "/firmware") >> fw;
      pruNameStates[to_wstring(name)] = to_wstring(state);
      pruNamePath[to_wstring(name)] = to_wstring(std::string(rproc.path()));
      pruNameFw[to_wstring(name)] = to_wstring(fw);
      pruList.push_back(to_wstring(name));
    }
  }

  Element Render() override {
    Elements elements, statusList, nameList, notifList, rightButtons,
        leftButtons;
    nameList.push_back(text(L" PRU ") | border);
    statusList.push_back(text(L" Status ") | border);
    notifList.push_back(text(L" Info ") | border);
    updateStatus();

    for (auto pru : pruList) {
      std::wstring fw = pruNameFw[pru], notif;
      if (std::filesystem::exists("/lib/firmware/" + to_string(fw))) {
        if (std::filesystem::file_size("/lib/firmware/" + to_string(fw)) == 0) {
          notif = L" Warning: " + fw + L" Empty";
        } else {
          notif = L" Loaded Firmware: " + fw;
        }
      } else {
        notif = L" Firmware Not found / Not Supported";
      }
      notifList.push_back(text(notif));
      nameList.push_back(text(pru));
      statusList.push_back(text(pruNameStates[pru]));
    }

    for (auto buttons : pruButtons) {
      leftButtons.push_back(buttons.first->Render());
    }
    for (auto buttons : pruButtons) {
      rightButtons.push_back(buttons.second->Render());
    }

    auto table1 = hbox({
        vbox(nameList),
        separator(),
        vbox(statusList),
        separator(),
        vbox(notifList),
    });
    auto table2 = hbox({
        vbox(leftButtons),
        separator(),
        vbox(rightButtons),
    });
    auto tabularLayout = vbox({
        table1 | hcenter,
        separator(),
        table2 | center,
    });
    elements.push_back(tabularLayout | flex | border);
    return vbox(elements);
  }

  Component pruPanel;
  std::vector<std::pair<Component, Component>> pruButtons;
  std::unordered_map<std::wstring, std::wstring>
      pruNameStates;  // PRU name - State
  std::unordered_map<std::wstring, std::wstring>
      pruNamePath;  // PRU name - Path
  std::unordered_map<std::wstring, std::wstring>
      pruNameFw;  // PRU name - Firmware
  std::vector<std::wstring> pruList;
};

namespace panel {
Panel PRU() {
  return Make<PRUImpl>();
}

}  // namespace panel
}  // namespace ui
