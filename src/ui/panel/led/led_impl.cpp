#include "ftxui/component/component.hpp"
#include "ftxui/dom/elements.hpp"
#include "ui/panel/panel.hpp"
#include "utils.hpp"

#define LEDS_PATH "/sys/class/leds/"

using namespace ftxui;

namespace ui {

class LedImpl : public PanelBase {
 public:
  LedImpl() {
    shell_helper("ls -C1 " LEDS_PATH, &result);
    while (result.length()) {
      auto pos = result.find('\n');
      if (pos != std::string::npos) {
        auto name = result.substr(0, pos);
        if (name != "\n")
          names.push_back(name);
        result.erase(0, pos + 1);
      }
    }

    on_time_vec.resize(names.size());
    multiplier_vec.resize(names.size());

    for (size_t i = 0; i < names.size(); i++) {
      auto name = names[i];
      shell_helper(("cat " LEDS_PATH + name + "/brightness").data(), &result);
      result.pop_back();
      if (result == "0") {
        states[name] = false;

      } else {
        states[name] = true;
      }

      toggle_buttons.push_back(
          Button(to_wstring(name), [=] { toggleLed(name); }));
      trigger_buttons.push_back(Button(L"Trigger" + to_wstring(name),
                                       [=] { setTriggerTime(name, i); }));
      time_on_sliders.push_back(Slider(L"Delay On Time", &on_time_vec[i],
                                       (float)0, (float)25, (float)1));
      multiplier_sliders.push_back(
          Slider(L"Multiplier", &multiplier_vec[i], 0, 40, 1));
    }
    Components toggle_component_h;
    for (size_t i = 0; i < names.size(); i++) {
      toggle_component_h.push_back(Container::Horizontal({
          toggle_buttons[i],
          trigger_buttons[i],
          multiplier_sliders[i],
          time_on_sliders[i],
      }));
    }
    Add(Container::Vertical(toggle_component_h));
  }
  ~LedImpl() = default;
  std::wstring Title() override { return L"USR LEDs"; }

 private:
  /* Toggle the "On/Off" status */
  void toggleLed(std::string name) {
    auto str = "echo '" + std::to_string(!(states[name])) + "' > " LEDS_PATH +
               name + "/brightness";
    shell_helper(str.c_str());
    states[name] = !states[name];
  }

  /* Enable trigger and set "delay_on" and "delay_off" */
  void setTriggerTime(std::string name, int i) {
    auto str = "echo 'timer' > " LEDS_PATH + name + "/trigger";
    shell_helper(str.c_str());
    str = "echo '" + std::to_string(int(on_time_vec[i] * multiplier_vec[i])) +
          "' > " LEDS_PATH + name + "/delay_on";
    shell_helper(str.c_str());
    str = "echo '" +
          std::to_string(int((25 - on_time_vec[i]) * multiplier_vec[i])) +
          "' > " LEDS_PATH + name + "/delay_off";
    shell_helper(str.c_str());
  }

  /* Store result of command */
  std::string result;

  /* Names of leds dependent on the board */
  std::vector<std::string> names;

  /* States of leds */
  std::unordered_map<std::string, bool> states;

  /* Timer "On" time */
  std::vector<float> on_time_vec;

  /* Multiplier for total time */
  std::vector<int> multiplier_vec;

  /* Toggle Led On/Off  */
  Components toggle_buttons;

  /* Set Trigger buttons */
  Components trigger_buttons;

  /* Sldiers for "On" time */
  Components time_on_sliders;

  /* Multipliers for "On" time */
  Components multiplier_sliders;
};

namespace panel {
Panel Led() {
  return Make<LedImpl>();
}

}  // namespace panel
}  // namespace ui
