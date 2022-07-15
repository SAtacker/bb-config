#include <vector>
#include <string>
#include <filesystem>
#include <fstream>

#include "process.hpp"
#include "ftxui/component/component.hpp"
#include "ftxui/dom/elements.hpp"
#include "ui/panel/panel.hpp"

const std::string PWM_FILE_PATH = "/sys/class/pwm/";

using namespace ftxui;

std::vector<std::string> FindPWMs() {
  std::vector<std::string> names;

  procxx::process ls{"ls"};
  ls.add_argument("-C1");
  ls.add_argument(PWM_FILE_PATH);
  ls.exec();

  std::string name;
  while (std::getline(ls.output(), name))
    if (name[3] == '-')
        names.push_back(name);
  return names;
}

namespace ui {
class DACImpl : public PanelBase {
    public:
        DACImpl() {
            for (auto name : FindPWMs()) {
                v_DAC_pin_.push_back(name);
            }
            
            Component page = Renderer(Container::Vertical({
                                  pwm_radiobox,
                                  Container::Vertical({
                                    slider_period,
                                    dropdown_unit
                                  }),
                                  slider_dutyCycle,
                                  polarity_radiobox,
                                  button,
                              }),
                              [&] {
                                return vbox({
                                    text("Select a LED:") | bold,
                                    pwm_radiobox->Render(),
                                    separator(),
                                    hbox(
                                        slider_period->Render() | vcenter | size(HEIGHT, EQUAL, 3) | xflex,
                                        dropdown_unit->Render() | size(WIDTH, EQUAL, 15) | size(HEIGHT, LESS_THAN, 8) | vscroll_indicator
                                    ),
                                    separator(),
                                    slider_dutyCycle->Render(),
                                    separator(),
                                    hbox(
                                      text("Period: ")| bold, 
                                      text(std::to_string(value_period)),
                                      text(unit_entries[select_unit]),
                                      text(" Duty Cycle: ")| bold, 
                                      text(std::to_string(value_dutyCycle)), 
                                      text("%")
                                    ),
                                    separator(),
                                    text("Select a Polarity:") | bold,
                                    polarity_radiobox->Render(),
                                    separator(),
                                    button->Render(),
                                }) | vscroll_indicator | frame;
                              });
            Add(page);
        }

        ~DACImpl() = default;

        std::string Title() override { return "DAC"; }
    
    private:
        std::vector<std::string> v_DAC_pin_;
        std::vector<std::string> v_polarity_ = {"normal", "inversed"};
        std::vector<std::string> unit_entries = {"s", "ms", "us", "ns"};
        int selected = 0, 
                selected_polarity = 0,
                value_period = 0, 
                select_unit = 0,
                value_dutyCycle = 0;
        Component pwm_radiobox = Radiobox(&v_DAC_pin_, &selected);
        Component slider_period = Slider("Period", &value_period, 0, 100, 1);
        Component slider_dutyCycle = Slider("Duty Cycle(%)", &value_dutyCycle, 0, 100, 1);
        Component dropdown_unit = Dropdown(&unit_entries, &select_unit);
        Component polarity_radiobox = Radiobox(&v_polarity_, &selected_polarity);
        Component button = Button("Trigger", [this] { TriggerPWM(); } );

        void TriggerPWM() {
          std::vector<long long> divider = {1000000000, 1000000, 1000, 1};
          std::string path_name = PWM_FILE_PATH + v_DAC_pin_[selected];

          long long period = value_period * divider[select_unit];
          std::ofstream(path_name + "/period") << period;

          float duty_cycle_f = (float) value_dutyCycle / 100 * (float) period;
          int duty_cycle = (int) duty_cycle_f;
          std::ofstream(path_name + "/duty_cycle") << duty_cycle;

          std::ofstream(path_name + "/polarity") << v_polarity_[selected_polarity];
          std::ofstream(path_name + "/enable") << "1";
        }
};

namespace panel {
Panel DAC() {
  return Make<DACImpl>();
}

}  // namespace panel   

}  // namespace ui