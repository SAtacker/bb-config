#include <functional> // for function
#include <memory>     // for allocator, unique_ptr
#include <string>     // for operator+, to_wstring

#include "ftxui/component/component.hpp"          // for Component, Compone...
#include "ftxui/component/container.hpp"          // for Container
#include "ftxui/component/event.hpp"              // for Event, Event::Escape
#include "ftxui/component/screen_interactive.hpp" // for ScreenInteractive
#include "ftxui/component/slider.hpp"             // for Slider
#include "ftxui/dom/elements.hpp"                 // for separator, operator|
#include "ftxui/screen/box.hpp"                   // for ftxui
#include "ftxui/screen/color.hpp"                 // for Color

using namespace ftxui;

namespace ui {

Element ColorTile(int red, int green, int blue);
Element ColorString(int red, int green, int blue);

class MyComponent : public Component {
public:
  MyComponent(int *red, int *green, int *blue, std::function<void(void)> quit);
  Element Render();
  bool OnEvent(Event event);

private:
  int *red_;
  int *green_;
  int *blue_;
  Container container_ = Container::Vertical();
  ComponentPtr slider_red_ = Slider(L"Red  :", red_, 0, 255, 1);
  ComponentPtr slider_green_ = Slider(L"Green:", green_, 0, 255, 1);
  ComponentPtr slider_blue_ = Slider(L"Blue :", blue_, 0, 255, 1);
  std::function<void(void)> quit_;
};
} // namespace ui
