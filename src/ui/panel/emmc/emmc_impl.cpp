#include "ftxui/dom/elements.hpp"
#include "ui/panel/panel.hpp"

using namespace ftxui;

namespace ui {

class EMMCImpl : public PanelBase {
 public:
  EMMCImpl() {
    Add(Renderer([] {
      return vbox({
          hbox(text(L"  SD card:["), gauge(0.5f), text(L"] (50%)")),
          hbox(text(L"EMMC card:["), gauge(0.75f), text(L"] (75%)")),
      });
      return gauge(0.75);
    }));
  }
  ~EMMCImpl() = default;
  std::wstring Title() override { return L"EMMC and MicroSD stats"; }
};

namespace panel {
Panel EMMC() {
  return Make<EMMCImpl>();
}

}  // namespace panel
}  // namespace ui
