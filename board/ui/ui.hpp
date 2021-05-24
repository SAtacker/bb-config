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

using namespace ftxui;

namespace ui {

class beagle_window {
 private:
  /* data */
  Component main_menu;

  ScreenInteractive* main_screen;
  std::vector<std::wstring> main_entries;
  std::vector<std::wstring> network_entries;
  std::vector<std::wstring> tab_entries;
  static int main_selected;
  static int network_selected;
  static int tab_index;
  static int depth;
  std::wstring captured_output;

 public:
  beagle_window();
  Component get_menu();
  ScreenInteractive* get_screen();
  int get_selected_number();
  std::wstring get_selected_option();
  void execute();
  std::wstring get_help_string();
  Elements set_output_window(std::wstring str);
  std::string manage_command(const char* cmd);
};

}  // namespace ui
