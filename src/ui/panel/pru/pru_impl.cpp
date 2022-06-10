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
  std::string info() const { return info_; }

 private:
  void Fetch() {
    std::ifstream(path_ + "/name") >> name_;
    std::ifstream(path_ + "/firmware") >> firmware_;
    std::ifstream(path_ + "/state") >> state_;
    if (std::filesystem::exists("/lib/firmware/" + firmware_)) {
      if (std::filesystem::file_size("/lib/firmware/" + firmware_) == 0) {
        info_ = " Warning: " + firmware_ + " Empty";
      } else {
        info_ = " Loaded Firmware: " + firmware_;
      }
    } else {
      info_ = " Firmware Not found / Not Supported";
    }
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
            "[Start]", [&] { StoreState("start"); }, opt),
        Button(
            "[Stop]", [&] { StoreState("stop"); }, opt),
    }));
  }

  std::string path_;
  std::string name_;
  std::string state_;
  std::string firmware_;
  std::string info_;
};

class PRUPanel : public PanelBase {
 public:
  PRUPanel() { BuildUI(); }
  std::string Title() override { return "PRU enable/disable"; }

 private:
  void BuildUI() {
    if (!std::filesystem::exists("/sys/class/remoteproc/"))
      return;
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
    Elements name_list = {text("PRU"), separator()};
    Elements firmware_list = {text("Firmware"), separator()};
    Elements state_list = {text("State"), separator()};
    Elements action_list = {text("Actions"), separator()};
    Elements info_list = {text("Info"), separator()};

    for (const auto& child : children_) {
      name_list.push_back(text(child->name()));
      firmware_list.push_back(text(child->firmware()));
      state_list.push_back(text(child->state()));
      action_list.push_back(child->Render());
      info_list.push_back(text(child->info()));
    }

    return window(text(" PRUS(s)  "), hbox({
                                          vbox(std::move(name_list)),
                                          separator(),
                                          vbox(std::move(firmware_list)),
                                          separator(),
                                          vbox(std::move(state_list)),
                                          separator(),
                                          vbox(std::move(action_list)) | flex,
                                          separator(),
                                          vbox(std::move(info_list)) | flex,
                                      }) | frame |
                                          flex);
  }

  std::vector<std::shared_ptr<Pru>> children_;
};

namespace panel {
Panel PRU() {
  return Make<PRUPanel>();
}

}  // namespace panel
}  // namespace ui
