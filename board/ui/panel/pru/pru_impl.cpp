#include "ftxui/dom/elements.hpp"
#include "ui/panel/panel.hpp"

using namespace ftxui;

namespace ui {

// A basic implementation of PanelBase with no internal logic.
class PRUImpl : public PanelBase {
 public:
  PRUImpl() {
    Add(Container::Vertical({
      Container::Horizontal({
        Button(L"PRU On", []{}),
        Button(L"PRU Off", []{}),
      }),
      Button(L"Upload", [] {}),
    }));
  }
  ~PRUImpl() = default;
  std::wstring Title() override { return L"PRU enable/disable"; }
};

namespace panel {
Panel PRU() { return Make<PRUImpl>(); }

}  // namespace panel
}  // namespace ui
