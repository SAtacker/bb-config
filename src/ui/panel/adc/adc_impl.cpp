#include <fstream>
#include <sstream>
#include <regex>
#include <cmath>
#include <functional>  
#include <memory>  
#include <chrono> 
#include <cstdlib>
#include <vector>
#include <filesystem>

#include "ftxui/component/component.hpp"
#include "ftxui/dom/elements.hpp"
#include "process.hpp"
#include "ui/panel/panel.hpp"

using namespace ftxui;
using namespace std::chrono_literals;

namespace ui {

const std::string Analog_Path = "/sys/bus/iio/devices/iio:device0";
const int Max_Analog = 4095;

std::vector<std::string> FindAnalogs() {
  std::vector<std::string> names;

  for (const auto& it : std::filesystem::directory_iterator(Analog_Path)) {
    std::stringstream ss(it.path());
    std::string name;

    while(ss.good()) {
      getline(ss, name, '/');
    }

    if ( std::regex_match(name, std::regex("(in_voltage)([0-9]+)(_raw)")) )
      names.push_back(name);
  }

  return names;
}

class Graph {
  public:
    std::vector<int> operator()(int width, int height) const {
      std::vector<int> output(width);
      for (int i = 0; i < width; ++i) {
        if ( i >= (int)data_vect.size() )
          break;

        float v = 0;
        v += (float) data_vect[i];
        v /= Max_Analog;
        v *= height;

        output[i] = static_cast<int>(v);
      }
      return output;
    }
    int shift = 0;

    void update() {
      int value;
      std::ifstream(Analog_Path + "/" + name_) >> value;
      data_vect.erase(data_vect.begin());
      data_vect.push_back(value);
    }

    void set_name(std::string input) {
      name_ = input;
    }

  private:
    std::vector<int> data_vect = std::vector<int>(200, 10);
    std::string name_;
};

class graphImpl : public ComponentBase {
  public:
    graphImpl(std::string name, int *tab) : name_(name), tab_(tab) {
      my_graph.set_name(name_);
      Add( Container::Vertical({button_}) );
    }

    std::string label() const { return name_; }

    Element Render() override {
      my_graph.update();

      return vbox({
          hbox({ 
            text(name_),
            text(" [Mhz]"),
          }) | hcenter,
          hbox({
              vbox({
                  text("4096 "),
                  filler(),
                  text("2048 "),
                  filler(),
                  text("0 "),
              }),
              graph(std::ref(my_graph)) | flex,
          }) | flex,
          button_->Render(),
      });
    }

  private:
    std::string name_;
    Graph my_graph;
    int *tab_;
    Component button_ = Button("Back", [this] { *tab_ = 1; });
};

class adcImpl : public PanelBase {
  public:
    adcImpl() {
      if (std::filesystem::exists(Analog_Path)) {
        for (auto name : FindAnalogs()) {
          analog_pin_.push_back(name);
          auto graph = std::make_shared<graphImpl>(name, &tab);
          children_.push_back(graph);
          graph_tab_->Add(graph);
        }
      }

      Add( Container::Tab( {
        Container::Vertical({
          radio_,
          button_,
        }),
        graph_tab_,
      }, &tab ));
    }

    ~adcImpl() override = default;

  private:
    std::string Title() override { return "ADC"; }
    int selected = 0;
    int tab = 0;
    std::vector<std::string> analog_pin_;
    std::vector<std::shared_ptr<graphImpl>> children_;  
    Component button_ = Button("Generate", [this] { tab = 1; } );
    Component radio_ = Radiobox(&analog_pin_, &selected);
    Component graph_tab_ =  Container::Vertical({}, &selected);

    Element Render() override {
      analog_pin_.clear();
      for (const auto& child : children_) {
        analog_pin_.push_back(child->label());
      }

      int i = 0;
      if (tab == 1) {
        for (const auto& child : children_) {
          if (i == selected) {
            return child->Render();
          }
          i++;
        }
      }

      return vbox({
        radio_->Render(),
        button_->Render(),
      }) | vscroll_indicator | frame;
    }
};

namespace panel {
Panel Adc() {
  return Make<adcImpl>();
}

}  // namespace panel

}  // namespace ui