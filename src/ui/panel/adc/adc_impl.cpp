#include "ftxui/component/component.hpp"
#include "ui/panel/panel.hpp"
#include "utils.hpp"
#include <cmath>   

using namespace ftxui;

namespace ui {

namespace {

class Graph {
 public:
  std::vector<int> operator()(int width, int height) const {
    std::vector<int> output(width);
    for (int i = 0; i < width; ++i) {
      float v = 0;
      v += 0.1f * sin((i + shift) * 0.1f);        // NOLINT
      v += 0.2f * sin((i + shift + 10) * 0.15f);  // NOLINT
      v += 0.1f * sin((i + shift) * 0.03f);       // NOLINT
      v *= height;                                // NOLINT
      v += 0.5f * height;                         // NOLINT
      output[i] = static_cast<int>(v);
    }
    return output;
  }
  int shift = 0;
};
 
std::vector<int> triangle(int width, int height) {
  std::vector<int> output(width);
  for (int i = 0; i < width; ++i) {
    output[i] = i % (height - 4) + 2;
  }
  return output;
}

Graph my_graph;

class adcImpl : public PanelBase {
 public:
  adcImpl() {
  }
  ~adcImpl() = default;
  std::string Title() override { return "ADC"; }

  Element Render() override {
    my_graph.shift++;
    return hbox({
        vbox({
            graph(std::ref(my_graph)),
            separator(),
            graph(triangle) | inverted,
        }) | flex,
        separator(),
        vbox({
            graph(std::ref(my_graph)) | color(Color::BlueLight),
            separator(),
            graph(std::ref(my_graph)) | color(Color::RedLight),
            separator(),
            graph(std::ref(my_graph)) | color(Color::YellowLight),
        }) | flex,
    });
  }
};

}  // namespace

namespace panel {
Panel Adc() {
  return Make<adcImpl>();
}

}  // namespace panel

}  // namespace ui