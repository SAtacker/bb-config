#ifndef BEAGLE_CONFIG_UI_PANEL_FOCUSABLE
#define BEAGLE_CONFIG_UI_PANEL_FOCUSABLE

namespace ui {

using namespace ftxui;
// A simple text highlighted when focused.
class Focusable : public ftxui::ComponentBase {
 public:
  Focusable(const std::wstring& title) : title_(title) {}
  Element Render() final {
    auto style = Focused() ? inverted : nothing;
    return text(title_) | style;
  }

 private:
  std::wstring title_;
};

}  // namespace ui

#endif /* end of include guard: BEAGLE_CONFIG_UI_PANEL_FOCUSABLE */
