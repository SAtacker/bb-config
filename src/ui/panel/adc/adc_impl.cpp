#include <fstream>
#include <sstream>
#include "ftxui/component/component.hpp"
#include "ftxui/dom/elements.hpp"
#include "process.hpp"
#include "ui/panel/panel.hpp"

#define ADC_PATH "/sys/bus/iio/devices/iio\\:device0/"

using namespace ftxui;

namespace ui {

std::vector<std::string> FindADCs() {
  std::vector<std::string> names;

  procxx::process ls{"ls"};
  ls.add_argument("-C1");
  ls.add_argument("/sys/bus/iio/devices/iio:device0/");
  ls.exec();

  std::string name;
  while (std::getline(ls.output(), name))
    names.push_back(name);
  return names;
}

class adcImpl : public PanelBase {
  public:
    adcImpl() {
      for (auto name : FindADCs()) {
        pin_.push_back(name);
      }
      Add(Container::Vertical({
        analog_radiobox_,
        sample_frequency_,
        button_generate_,
      }));
    }
    ~adcImpl() = default;

  private:
    Element Render() override {
      return vbox({
                text("Select an Analogue Pin:"),
                hbox(text(" "), analog_radiobox_->Render()),
                separator(),
                hbox(text(" "), sample_frequency_->Render()),
                separator(),
                button_generate_->Render(),
              })|
              vscroll_indicator | frame;
    }

    std::string Title() override { return "ADC"; }

    int selected_analog_pin = 0;
    int selected_frequency = 0;
    std::vector<std::string> pin_;
    std::vector<std::string> fre_ {"1kHz", "2kHz"};

    Component analog_radiobox_ = Radiobox(&pin_, &selected_analog_pin);
    Component sample_frequency_ = Dropdown(&fre_, &selected_frequency);
    Component button_generate_ = Button("Generate", [this] {});
};

namespace panel {
Panel Adc() {
  return Make<adcImpl>();
}

}  // namespace panel

}  // namespace ui
