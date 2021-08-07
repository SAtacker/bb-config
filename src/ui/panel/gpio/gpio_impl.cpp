#include <filesystem>
#include <fstream>
#include "ftxui/component/component.hpp"
#include "ftxui/dom/elements.hpp"
#include "ui/panel/panel.hpp"

using namespace ftxui;

namespace ui {

class Gpio : public ComponentBase {
 public:
  Gpio(std::string path) : path_(path) {
    Fetch();
    BuildUI();
  }

  std::string label() const { return label_; }
  std::string direction() const { return direction_; }
  std::string edge() const { return edge_; }
  std::string value() const { return value_; }
  std::string active_low() const { return active_low_; }

 private:
  void Fetch() {
    std::ifstream(path_ + "/label") >> label_;
    std::ifstream(path_ + "/edge") >> edge_;
    std::ifstream(path_ + "/direction") >> direction_;
    std::ifstream(path_ + "/value") >> value_;
    std::ifstream(path_ + "/active_low") >> active_low_;
  }

  void StoreDirection(std::string direction) {
    std::ofstream(path_ + "/direction") << direction;
    Fetch();
  };

  void StoreEdge(std::string edge) {
    std::ofstream(path_ + "/edge") << edge;
    Fetch();
  };

  void StoreValue(std::string value) {
    std::ofstream(path_ + "/value") << value;
    Fetch();
  };

  void StoreActiveLow(std::string active_low) {
    std::ofstream(path_ + "/active_low") << active_low;
    Fetch();
  };

  void BuildUI() {
    ButtonOption opt;
    opt.border = false;
    Add(Container::Horizontal({
        Button(
            L"[IN]", [&] { StoreDirection("in"); }, opt),
        Button(
            L"[OUT]", [&] { StoreDirection("out"); }, opt),
        Button(
            L"[0]", [&] { StoreValue("0"); }, opt),
        Button(
            L"[1]", [&] { StoreValue("1"); }, opt),
        Button(
            L"[Active Low]", [&] { StoreActiveLow("0"); }, opt),
        Button(
            L"[Active High]", [&] { StoreActiveLow("1"); }, opt),
        Button(
            L"[Edge +ve]", [&] { StoreEdge("positive"); }, opt),
        Button(
            L"[Edge -ve]", [&] { StoreEdge("negative"); }, opt),
        Button(
            L"[Edge none]", [&] { StoreEdge("none"); }, opt),
    }));
  }

  std::string path_;
  std::string label_;
  std::string direction_;
  std::string edge_;
  std::string value_;
  std::string active_low_;
};

const std::vector<std::wstring> toggle_entries = {
    L"Input",
    L"Output",
};

class GPIOImpl : public PanelBase {
 public:
  GPIOImpl() { BuildUI(); }
  std::wstring Title() override { return L"GPIO"; }

 private:
  void BuildUI() {
    Component vertical_list = Container::Vertical({});
    for (const auto& it :
         std::filesystem::directory_iterator("/sys/class/gpio/")) {
      auto pru = std::make_shared<Gpio>(it.path());
      children_.push_back(pru);
      vertical_list->Add(pru);
    }
    Add(vertical_list);
  }

  Element Render() override {
    Elements name_list = {text(L"Name"), separator()};
    Elements edge_list = {text(L"Edge"), separator()};
    Elements direction_list = {text(L"Direction"), separator()};
    Elements value_list = {text(L"Value"), separator()};
    Elements active_state_list = {text(L"Active L/H"), separator()};
    Elements action_list = {text(L"Actions"), separator()};

    for (const auto& child : children_) {
      name_list.push_back(text(to_wstring(child->label())));
      edge_list.push_back(text(to_wstring(child->edge())));
      direction_list.push_back(text(to_wstring(child->direction())));
      value_list.push_back(text(to_wstring(child->value())));
      active_state_list.push_back(text(to_wstring(child->active_low())));
      action_list.push_back(hbox(child->Render()));
    }

    return window(text(L" GPIO Status "),
                  hbox({
                      vbox(std::move(name_list)),
                      separator(),
                      vbox(std::move(edge_list)),
                      separator(),
                      vbox(std::move(direction_list)),
                      separator(),
                      vbox(std::move(value_list)),
                      separator(),
                      vbox(std::move(active_state_list)),
                      separator(),
                      vbox(std::move(action_list)) | flex,
                      separator(),
                  }) | frame |
                      flex);
  }

  std::vector<std::shared_ptr<Gpio>> children_;
};

namespace panel {
Panel GPIO() {
  return Make<GPIOImpl>();
}
}  // namespace panel
}  // namespace ui
