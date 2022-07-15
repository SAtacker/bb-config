#include <vector>
#include <string>
#include <filesystem>

#include "ftxui/component/component.hpp"
#include "ftxui/dom/elements.hpp"
#include "ui/panel/panel.hpp"

const std::string PWM_FILE_PATH = "/sys/class/pwm";

using namespace ftxui;

namespace ui {
class DACImpl : public PanelBase {
    public:
        DACImpl() {
            for (const auto& it : std::filesystem::directory_iterator(PWM_FILE_PATH)) {
                std::string pwm_path = it.path();
                if (pwm_path.find("pwm-0"))
                    v_DAC_pin_.push_back(pwm_path);
            }
            
            Component page = Renderer(Container::Vertical({
                                  Container::Horizontal({
                                      pwm_radiobox,
                                  }),
                              }),
                              [&] {
                                return vbox({
                                    pwm_radiobox->Render() | flex,
                                });
                              });
            Add(page);
        }

        ~DACImpl() = default;

        std::string Title() override { return "DAC"; }
    
    private:
        std::vector<std::string> v_DAC_pin_;
        int selected = 0;
        Component pwm_radiobox = Radiobox(&v_DAC_pin_, &selected);

};

namespace panel {
Panel DAC() {
  return Make<DACImpl>();
}

}  // namespace panel   

}  // namespace ui