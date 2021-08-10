#include <filesystem>
#include <fstream>
#include "ftxui/component/component.hpp"
#include "ftxui/dom/elements.hpp"
#include "ui/panel/panel.hpp"

using namespace ftxui;

namespace ui {

const std::vector<std::string> iOentries = {
    "IN",
    "OUT",
};

const std::vector<std::string> valueEntries = {
    "0",
    "1",
};

const std::vector<std::string> activeEntries = {
    "Low",
    "High",
};

const std::vector<std::string> edgeEntries = {
    "Pos (+ve)",
    "Neg (-ve)",
    "Any",
    "None",
};

class Gpio : public ComponentBase {
 public:
  Gpio(std::string path, int* tab, int* next, int* limit) : path_(path) {
    limit_ = limit;
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

  void handleDirection() {
    if (ioTog == 0)
      StoreDirection("in");
    else
      StoreDirection("out");
  }

  void handleValue() {
    if (valTog == 0)
      StoreValue("0");
    else
      StoreValue("1");
  }

  void handleActiveLow() {
    if (activeTog == 0)
      StoreActiveLow("0");
    else
      StoreActiveLow("1");
  }

  void handleEdge() {
    switch (edgeTog) {
      case 0:
        StoreEdge("rising");
        break;
      case 1:
        StoreEdge("falling");
        break;
      case 2:
        StoreEdge("both");
        break;
      case 3:
        StoreEdge("none");
        break;
    }
  }

  void BuildUI() {
    ToggleOption iOtoggleOpt;
    ToggleOption valueToggleOpt;
    ToggleOption activeToggleOpt;
    ToggleOption edgeToggleOpt;
    iOtoggleOpt.on_enter = [&] { handleDirection(); };
    valueToggleOpt.on_enter = [&] { handleValue(); };
    activeToggleOpt.on_enter = [&] { handleActiveLow(); };
    edgeToggleOpt.on_enter = [&] { handleEdge(); };
    ioToggle = Toggle(&iOentries, &ioTog, iOtoggleOpt);
    valToggle = Toggle(&valueEntries, &valTog, valueToggleOpt);
    activeToggle = Toggle(&activeEntries, &activeTog, activeToggleOpt);
    edgeToggle = Toggle(&edgeEntries, &edgeTog, edgeToggleOpt);
    Component actions = Renderer(
        Container::Vertical({
            ioToggle,
            valToggle,
            activeToggle,
            edgeToggle,
        }),
        [&] {
          return vbox({
              text(label() + " Status "),
              hbox(text(" * Direction       : "), text(direction())),
              hbox(text(" * Value           : "), text(value())),
              hbox(text(" * Active Low      : "), text(active_low())),
              hbox(text(" * Edge            : "), text(edge())),
              text(" Actions "),
              hbox(text(" * Direction       : "), ioToggle->Render()),
              hbox(text(" * Value           : "), valToggle->Render()),
              hbox(text(" * Active Low      : "), activeToggle->Render()),
              hbox(text(" * Edge            : "), edgeToggle->Render()),
          });
        });
    Add(Container::Vertical(
        {actions, Container::Horizontal({
                      Button("Back to Menu", [&] { *tab_ = 0; }),
                      Button("Prev",
                             [&] {
                               (*next_)--;
                               if (*next_ < 0) {
                                 *next_ = 0;
                               }
                             }),
                      Button("Next",
                             [&] {
                               (*next_)++;
                               if (*limit_ < *next_) {
                                 *next_ = 0;
                               }
                             }),
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
  int* limit_;
  int ioTog;
  int activeTog;
  int valTog;
  int edgeTog;
  Component ioToggle;
  Component activeToggle;
  Component valToggle;
  Component edgeToggle;
};

class GPIOImpl : public PanelBase {
 public:
  GPIOImpl() { BuildUI(); }
  std::string Title() override { return "GPIO"; }

 private:
  void BuildUI() {
    MenuOption menuOpt;
    menuOpt.on_enter = [&] { tab = 1; };
    gpio_menu = Menu(&gpio_names, &selected, menuOpt);
    gpio_individual = Container::Vertical({}, &selected);
    for (const auto& it :
         std::filesystem::directory_iterator("/sys/class/gpio/")) {
      std::string gpio_path = it.path();
      if (gpio_path.find("gpiochip") == std::string::npos &&
          gpio_path.find("gpio") != std::string::npos) {
        auto gpio = std::make_shared<Gpio>(gpio_path, &tab, &selected, &limit);
        if (gpio->label().find("P") != std::string::npos) {
          children_.push_back(gpio);
          gpio_individual->Add(gpio);
          limit++;
        }
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
    gpio_names.clear();
    for (const auto& child : children_) {
      gpio_names.push_back(child->label());
    }

    int i = 0;
    if (tab == 1)
      for (const auto& child : children_) {
        if (i == selected) {
          return child->Render();
        }
        i++;
      }

    return window(text("GPIO Menu"), gpio_menu->Render() | frame | flex);
  }

  std::vector<std::shared_ptr<Gpio>> children_;
  std::vector<std::string> gpio_names;
  Component gpio_menu;
  Component gpio_individual;
  int selected = 0;
  int tab = 0;
  int limit = 0;
};

namespace panel {
Panel GPIO() {
  return Make<GPIOImpl>();
}
}  // namespace panel
}  // namespace ui
