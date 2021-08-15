#include "ftxui/component/component.hpp"
#include "process.hpp"
#include "ui/panel/panel.hpp"

using namespace ftxui;

namespace ui {

namespace {

class displayImpl : public PanelBase {
 public:
  displayImpl(){};
  ~displayImpl() = default;
  std::string Title() override { return "Display"; }

 private:
};

}  // namespace

namespace panel {
Panel display() {
  return Make<displayImpl>();
}

}  // namespace panel

}  // namespace ui