#include "ftxui/dom/elements.hpp"
#include "ui/panel/panel.hpp"

using namespace ftxui;

namespace ui {

class PlaceHolderImpl : public PanelBase {
 public:
  PlaceHolderImpl(std::wstring title) : title_(title) {}
  ~PlaceHolderImpl(){};
  std::wstring Title() override { return title_; }
  Element Render() override {
    auto style = Focused() ? inverted : nothing;
    return text(L"Not implemented") | style;
  }

 private:
  std::wstring title_;
};

namespace panel {
Panel PlaceHolder(const std::wstring& title) {
  return Make<PlaceHolderImpl>(title);
}

}  // namespace panel
}  // namespace ui
