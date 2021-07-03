#include "ftxui/component/component.hpp"
#include "ftxui/dom/elements.hpp"
#include "ui/panel/panel.hpp"

using namespace ftxui;

namespace ui {

class ICSImpl : public PanelBase {
 public:
  ICSImpl() {
    Add(Container::Horizontal({
        Button(L"ICS On", [] {}),
        Button(L"ICS Off", [] {}),
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
