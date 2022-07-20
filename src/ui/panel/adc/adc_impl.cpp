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
        v += data_vect[i];
        v /= Max_Analog;
        v *= height;

        output[i] = static_cast<int>(v);
      }
      return output;
    }
    int shift = 0;

    void update() {
      int value;
      std::ifstream(Analog_Path + name_) >> value;
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

class adcImpl : public PanelBase {
  public:
    adcImpl() {
      for (auto name : FindAnalogs()) {
        analog_pin_.push_back(name);
      }

      Add( Container::Vertical( {
        radio_,
      }));
    }

    ~adcImpl() override = default;

  private:
    std::string Title() override { return "ADC"; }
    int selected = 0;
    std::vector<std::string> analog_pin_;  
    Graph my_graph;
    Component button_ = Button("Button", [this] {} );
    Component radio_ = Radiobox(&analog_pin_, &selected); 

    Element Render() override {
      my_graph.shift++;
      my_graph.update();
      const auto sleep_time = 0.03s;
      std::this_thread::sleep_for(sleep_time);

      auto graph_page = vbox({
          text("Analog [Mhz]") | hcenter,
          hbox({
              vbox({
                  text("4095 "),
                  filler(),
                  text("2048 "),
                  filler(),
                  text("0 "),
              }),
              graph(std::ref(my_graph)) | flex,
          }) | flex,
      });

      return vbox({
        radio_->Render(),
        graph_page | flex,
      }) | vscroll_indicator | frame;
    }
};

namespace panel {
Panel Adc() {
  return Make<adcImpl>();
}

}  // namespace panel

}  // namespace ui