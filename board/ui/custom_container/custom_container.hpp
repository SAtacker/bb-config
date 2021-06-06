#include <functional>        // for function
#include <initializer_list>  // for initializer_list
#include <memory>            // for __shared_ptr_access, shared_ptr, allocator
#include <string>            // for wstring, basic_string
#include <vector>            // for vector

#include "ftxui/component/captured_mouse.hpp"  // for ftxui
#include "ftxui/component/component.hpp"       // for Menu, Horizontal, Renderer
#include "ftxui/component/component_base.hpp"  // for ComponentBase
#include "ftxui/component/menu.hpp"            // for MenuBase
#include "ftxui/component/screen_interactive.hpp"  // for ScreenInteractive, Component
#include "ftxui/dom/elements.hpp"  // for operator|, Element, separator, bgcolor, color, flex, Decorator, bold, hbox, border, dim
#include "ftxui/screen/color.hpp"  // for Color, Color::Blue, Color::BlueLight, Color::Red, Color::Yellow             // for Color

#pragma once

using namespace ftxui;

namespace custom_container_component {

Component panel(Components components,
                std::vector<std::wstring> titles,
                int sel = 0,
                int* external = nullptr);

class panel_base : public ComponentBase {
 private:
  std::vector<std::wstring> titles;

  Component panel;
  Components components;

  int* external = nullptr;

  /* 0 for vertical and 1 for horizontal */
  int sel;

 public:
  // Access this interface from a Component
  static panel_base* From(Component component);

  panel_base(Components components,
             std::vector<std::wstring> titles,
             int sel,
             int*);
  ~panel_base() override = default;

  Element Render() override;
};

}  // namespace custom_container_component