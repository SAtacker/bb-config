#include "ftxui/dom/elements.hpp"
#include "ui/panel/panel.hpp"

using namespace ftxui;

namespace ui {

class PlaceHolderImpl : public PanelBase {
 public:
  PlaceHolderImpl(std::string title) : title_(title) {}
  ~PlaceHolderImpl(){};
  std::string Title() override { return title_; }
  Element Render() override {
    auto style = Focused() ? inverted : nothing;
    return text("Not implemented") | style;
  }

 private:
  std::string title_;
};

namespace panel {
Panel PlaceHolder(const std::string& title) {
  return Make<PlaceHolderImpl>(title);
}

}  // namespace panel
}  // namespace ui
