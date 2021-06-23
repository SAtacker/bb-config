#ifndef BEAGLE_CONFIG_PANEL_HPP
#define BEAGLE_CONFIG_PANEL_HPP

#include <memory>
#include "ftxui/component/component.hpp"

namespace ui {
using namespace ftxui;

// A PanelBase is a Component with an associated title.
class PanelBase : public ftxui::ComponentBase {
 public:
  virtual ~PanelBase() {}
  virtual std::wstring Title() = 0;
};

using Panel = std::shared_ptr<PanelBase>;

namespace panel {
Panel PlaceHolder(const std::wstring& title);
Panel PRU();
Panel GPIO();
Panel ICS();
Panel EMMC();
}  // namespace panel
}  // namespace ui

#endif /* end of include guard: BEAGLE_CONFIG_PANEL_HPP */
