#include <filesystem>
#include <fstream>
#include <unordered_map>
#include "ftxui/dom/elements.hpp"
#include "ui/panel/panel.hpp"

using namespace ftxui;

namespace ui {

class Pru : public ComponentBase {
 public:
  Pru(std::string path) : path_(path) {
    Fetch();
    BuildUI();
  }

  std::string name() const { return name_; }
  std::string firmware() const { return firmware_; }
  std::string state() const { return state_; }

 private:
  void Fetch() {
    std::ifstream(path_ + "/name") >> name_;
    std::ifstream(path_ + "/firmware") >> firmware_;
    std::ifstream(path_ + "/state") >> state_;
  }

  void StoreState(std::string state) {
    std::ofstream(path_ + "/state") << state;
    Fetch();
  };

  void BuildUI() {
    ButtonOption opt;
    opt.border = false;
    Add(Container::Horizontal({
        Button(
            L"[Start]", [&] { StoreState("start"); }, opt),
        Button(
            L"[Stop]", [&] { StoreState("stop"); }, opt),
    }));
  }

  std::string path_;
  std::string name_;
  std::string state_;
  std::string firmware_;
};

class PRUPanel : public PanelBase {
 public:
  PRUPanel() {
    BuildUI();
  }
  std::wstring Title() override { return L"PRU enable/disable"; }

 private:
  void BuildUI() {
    Component vertical_list = Container::Vertical({});
    for (const auto& it :
         std::filesystem::directory_iterator("/sys/class/remoteproc/")) {
      auto pru = std::make_shared<Pru>(it.path());
      children_.push_back(pru);
      vertical_list->Add(pru);
    }
    Add(vertical_list);
  }

  Element Render() override {
    Elements name_list = {text(L"PRU"), separator()};
    Elements firmware_list = {text(L"Firmware"), separator()};
    Elements state_list = {text(L"State"), separator()};
    Elements action_list = {text(L"Actions"), separator()};

    for (const auto& child : children_) {
      name_list.push_back(text(to_wstring(child->name())));
      firmware_list.push_back(text(to_wstring(child->firmware())));
      state_list.push_back(text(to_wstring(child->state())));
      action_list.push_back(child->Render());
    }

    return hbox({
               vbox(std::move(name_list)),
               separator(),
               vbox(std::move(firmware_list)),
               separator(),
               vbox(std::move(state_list)),
               separator(),
               vbox(std::move(action_list)) | flex,
           }) |
           border;
  }

  std::vector<std::shared_ptr<Pru>> children_;
};

namespace panel {
Panel PRU() {
  return Make<PRUPanel>();
}

}  // namespace panel
}  // namespace ui
