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
const float Max_Voltage = 1.8;

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

// Graph class for handling how value stores and display in graph
class Graph {
  public:
    std::vector<int> operator()(int width, int height) const {
      std::vector<int> output(width);
      for (int i = 0; i < width; ++i) {
        if ( i >= (int)data_vect.size() )
          break;

        float v = 0;
        v += (float) data_vect[i];
        v -= hScale_start_;
        v /= Max_Analog;
        v *= height;

        for (int j = 0; j < sample_; j++) {
          if ( i*sample_+j >= width )
            break;
          output[i*sample_+j] = static_cast<int>(v);
        }
      }
      return output;
    }
    int shift = 0;

    void update() {
      int value;
      std::ifstream(Analog_Path + "/" + name_) >> value;
      data_vect.pop_back();
      data_vect.insert(data_vect.begin(), value);
    }

    // Update name for graph page
    void set_name(std::string input) {
      name_ = input;
    }

    // change the value for x-axis scale
    void set_sample(int sample) {
      sample_ = sample;
    }

    // Clear all the stored data
    void reset() {
      for (auto &item : data_vect) {
        item = 0;
      }
    }

  private:
    std::vector<int> data_vect = std::vector<int>(400, 0);
    std::string name_;
    int sample_;
    int hScale_start_;
    int hScale_end_;
};

// graphImpl class handles the page UI for the graph
class graphImpl : public ComponentBase {
  public:
    graphImpl(std::string name, int *tab, ScreenInteractive* screen) 
              : name_(name), tab_(tab), screen_(screen) {
      my_graph.set_name(name_);
      Add( Container::Vertical({
        Container::Horizontal({
          x_scaleDown_,
          x_scaleUp_,
        }),
        Container::Horizontal({
          button_,
          reset_,
        }),
      }) 
      );

      Update();
    }

    ~graphImpl() {
      // Clear all actuive threads before exit
      refresh_ui_continue_ = false;
      graph_update_.join();
    };

    std::string label() const { return name_; }

    Element Render() override {
      return vbox({
          hbox({
            vbox({
              text("scale: " + std::to_string(sample_)),
              hbox({
                x_scaleDown_->Render(),
                x_scaleUp_->Render(),
              }),
            }),
          }),
          separator(),
          hbox({ 
            text(name_),
            text(" [Hz]"),
          }) | hcenter,
          hbox({
              vbox({
                  text("1.8v "),
                  filler(),
                  text("0.9v "),
                  filler(),
                  text("0.0v "),
              }),
              graph(std::ref(my_graph)) | flex,      
          }) | flex,
          hbox({
              text(std::to_string(0/sample_) + "Hz "),
              filler(),
              text(std::to_string(150/sample_) + "Hz "),
              filler(),
              text(std::to_string(300/sample_) + "Hz "),
          }),
          separator(),
          hbox({
            button_->Render() | flex,
            reset_->Render() | flex,
          }),
      });
    }

  private:
    // Handle auto refresh the page with custome event
    void Update() {
      refresh_ui_continue_ = true;

      graph_update_ = std::thread([&] {
        while(refresh_ui_continue_) {
          using namespace std::chrono_literals;
          std::this_thread::sleep_for(1s);
          screen_->Post([&] { my_graph.update(); });
          screen_->Post(Event::Custom);
          my_graph.set_sample(sample_);
        }
      });
    }

    // Update the x-axis scale 
    void updateSample(int value) {
      if (value > 0 && value <= 10)
        sample_ = value; 
    }

    std::string name_;
    Graph my_graph;
    int *tab_;
    int sample_ = 1;
    float yScale_start_ = 0.f;
    float yScale_end_ = Max_Voltage;
    std::thread graph_update_;
    ScreenInteractive* screen_;
    std::atomic<bool> refresh_ui_continue_;
    Component button_ = Button("Back", [this] { *tab_ = 0; });
    Component x_scaleUp_ = Button("Increase", [&] { updateSample(sample_+1); });
    Component x_scaleDown_ = Button("Decress", [&] { updateSample(sample_-1); });
    Component reset_ = Button("Reset", [&] { my_graph.reset(); });
};

class adcImpl : public PanelBase {
  public:
    adcImpl(ScreenInteractive* screen) : screen_(screen) {
      // Get all the analog pin and create a graph page
      if (std::filesystem::exists(Analog_Path)) {
        for (auto name : FindAnalogs()) {
          // Store in a vector
          analog_pin_.push_back(name);
          // Create graph page
          auto graph = std::make_shared<graphImpl>(name, &tab, screen);
          children_.push_back(graph);
          graph_tab_->Add(graph);
        }
      }

      Add( Container::Tab( {
        // Home page (select pin)
        Container::Vertical({
          radio_,
          button_,
        }),
        // Graph page
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
    ScreenInteractive* screen_;
    std::thread graph_update_;
    bool refresh_ui_continue_;
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
        text("Select Analog Pin :"),
        radio_->Render(),
        separator(),
        button_->Render(),
      }) | vscroll_indicator | frame;
    }
};

namespace panel {
Panel ADC(ScreenInteractive* screen) {
  return Make<adcImpl>(screen);
}

}  // namespace panel

}  // namespace ui