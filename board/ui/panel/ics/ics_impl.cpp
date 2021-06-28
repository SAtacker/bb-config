#include "command_handler/command_handler.hpp"
#include "ftxui/component/component.hpp"
#include "ftxui/dom/elements.hpp"
#include "ui/panel/panel.hpp"

using namespace ftxui;

namespace ui {

class ICSImpl : public PanelBase {
 private:
  command_handler::cmd_h cmd;

 public:
  ICSImpl() {
    Add(Container::Vertical({
        Container::Horizontal({
            Button(L"ICS On", [&] { cmd.set_route_add_rem(true); }),
            Button(L"ICS Off", [&] { cmd.set_route_add_rem(false); }),
        }),
        Container::Horizontal({
            Input(L" Gateway ", cmd.def_gw_str),
            Input(L" DNS 1 ", cmd.dns_1_str),
            Input(L" DNS 2 ", cmd.dns_2_str),
        }),
    }));
  }
  ~ICSImpl() = default;
  std::wstring Title() override {
    return L"Internet Sharing and Client Configuration";
  }
};

namespace panel {
Panel ICS() {
  return Make<ICSImpl>();
}

}  // namespace panel
}  // namespace ui
