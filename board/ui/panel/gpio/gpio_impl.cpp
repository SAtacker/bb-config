#include "ftxui/component/component.hpp"
#include "ftxui/dom/elements.hpp"
#include "ui/panel/panel.hpp"

using namespace ftxui;

namespace ui {

const std::vector<std::wstring> toggle_entries = {
    L"Input",
    L"Output",
};

class GPIOImpl : public PanelBase {
 public:
  std::wstring Title() override { return L"GPIO configuration"; }
  GPIOImpl() { Add(renderer); }
  ~GPIOImpl() = default;

 private:
  int gpio_selection[5] = {false, false, true, false, false};
  Component gpio[5] = {
      Radiobox(&toggle_entries, &gpio_selection[0]),
      Radiobox(&toggle_entries, &gpio_selection[1]),
      Radiobox(&toggle_entries, &gpio_selection[2]),
      Radiobox(&toggle_entries, &gpio_selection[3]),
      Radiobox(&toggle_entries, &gpio_selection[4]),
  };
  Component layout =
      Container::Vertical({gpio[0], gpio[1], gpio[2], gpio[3], gpio[4]});
  Component renderer = Renderer(layout, [this] {
    return vbox({
        text(L"GPIO:"),
        hbox(text(L" [1]: "), gpio[0]->Render() | border),
        hbox(text(L" [2]: "), gpio[1]->Render() | border),
        hbox(text(L" [3]: "), gpio[2]->Render() | border),
        hbox(text(L" [4]: "), gpio[3]->Render() | border),
        hbox(text(L" [5]: "), gpio[4]->Render() | border),
    });
  });
};

namespace panel {
Panel GPIO() {
  return Make<GPIOImpl>();
}
}  // namespace panel
}  // namespace ui
