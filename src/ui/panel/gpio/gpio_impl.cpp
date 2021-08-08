#include <filesystem>
#include <fstream>
#include "ftxui/component/component.hpp"
#include "ftxui/dom/elements.hpp"
#include "ui/panel/panel.hpp"

using namespace ftxui;

namespace ui {

class Gpio : public ComponentBase {
 public:
  Gpio(std::string path, int* tab, int* next) : path_(path) {
    tab_ = tab;
    next_ = next;
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
    Add(Container::Vertical(
        {Container::Horizontal({
             Button(L"IN", [&] { StoreDirection("in"); }),
             Button(L"OUT", [&] { StoreDirection("out"); }),
             Button(L"Value 0", [&] { StoreValue("0"); }),
             Button(L"Value 1", [&] { StoreValue("1"); }),
             Button(L"Active Low", [&] { StoreActiveLow("0"); }),
             Button(L"Active High", [&] { StoreActiveLow("1"); }),
             Button(L"Edge +ve", [&] { StoreEdge("rising"); }),
             Button(L"Edge -ve", [&] { StoreEdge("falling"); }),
             Button(L"Edge Any", [&] { StoreEdge("both"); }),
             Button(L"Edge none", [&] { StoreEdge("none"); }),
         }),
         Container::Horizontal({
             Button(L"Back to Menu", [&] { *tab_ = 0; }),
             Button(L"Prev",
                    [&] {
                      (*next_)--;
                      if (*next_ < 0) {
                        *next_ = 0;
                      }
                    }),
             Button(L"Next", [&] { (*next_)++; }),
         })}));
  }

  std::string path_;
  std::string label_;
  std::string direction_;
  std::string edge_;
  std::string value_;
  std::string active_low_;
  int* tab_;
  int* next_;
};

class GPIOImpl : public PanelBase {
 public:
  GPIOImpl() { BuildUI(); }
  std::wstring Title() override { return L"GPIO"; }

 private:
  void BuildUI() {
    MenuOption menuOpt;
    menuOpt.on_enter = [&] { tab = 1; };
    gpio_menu = Menu(&gpio_names, &selected, menuOpt);
    gpio_individual = Container::Vertical({});
    for (const auto& it :
         std::filesystem::directory_iterator("/sys/class/gpio/")) {
      std::string gpio_path = it.path();
      if (gpio_path.find("gpiochip") == std::string::npos &&
          gpio_path.find("gpio") != std::string::npos) {
        auto gpio = std::make_shared<Gpio>(gpio_path, &tab, &selected);
        children_.push_back(gpio);
        gpio_individual->Add(gpio);
      }
    }

    Add(Container::Tab(
        {
            gpio_menu,
            gpio_individual,
        },
        &tab));
  }

  Element Render() override {
    Elements name_list = {text(L"Name"), separator()};
    Elements edge_list = {text(L"Edge"), separator()};
    Elements direction_list = {text(L"Direction"), separator()};
    Elements value_list = {text(L"Value"), separator()};
    Elements active_state_list = {text(L"Active L/H"), separator()};
    Elements action_list = {text(L"Actions"), separator()};

    gpio_names.clear();
    for (const auto& child : children_) {
      gpio_names.push_back(to_wstring(child->label()));
    }

    Element selected_gpio;
    int i = 0;
    if (tab == 1)
      for (const auto& child : children_) {
        if (i == selected) {
          name_list.push_back(text(to_wstring(child->label())));
          edge_list.push_back(text(to_wstring(child->edge())));
          direction_list.push_back(text(to_wstring(child->direction())));
          value_list.push_back(text(to_wstring(child->value())));
          active_state_list.push_back(text(to_wstring(child->active_low())));
          action_list.push_back(hbox(child->Render()));
          selected_gpio = vbox({
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
              }) | flex,
              separator(),
              hbox({
                  vbox(std::move(action_list)),
                  separator(),
              }) | flex,
          });
          return selected_gpio;
        }
        i++;
      }

    return window(text(L"GPIO Menu"), gpio_menu->Render() | frame | flex);
  }

  std::vector<std::shared_ptr<Gpio>> children_;
  std::vector<std::wstring> gpio_names;
  Component gpio_menu;
  Component gpio_individual;
  int selected = 0;
  int tab = 0;
};

namespace panel {
Panel GPIO() {
  return Make<GPIOImpl>();
}
}  // namespace panel
}  // namespace ui
