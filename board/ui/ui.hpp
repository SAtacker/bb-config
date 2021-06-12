#include "custom_container/custom_container.hpp"

using namespace ftxui;

namespace ui {

class checkbox_bool {
 public:
  checkbox_bool() : checked(false){};
  bool checked;
};

class beagle_window {
 private:
  /* This is the main window which appears before anything */
  Component main_window;

  /* A tab container renders the component based on the current selected menu */
  Component tab_container;

  /* A horizontal component containing menu component and tab container */
  Component container;

  /* This is the top level menu and component on the left */
  Component tab_menu;

  /* It's a vector of components on the right-hand side of window */
  std::vector<Component> right_component_1;
  std::vector<Component> right_component_2;
  std::vector<Component> right_component_3;

  /* This stores the selection of submenu */
  std::vector<int> right_component_selection;
  std::vector<checkbox_bool> gpio_selection{false};

  /* It stores the current top level menu selected */
  int tab_selected;
  int tab_1_selected;
  int tab_2_selected;
  int tab_3_selected;

  /* Sliders */
  int emmc_slider;

  /* It stores the entries for the right menu component */
  std::vector<std::wstring> tab_3_entries;
  std::vector<std::wstring> tab_2_entries;
  std::vector<std::wstring> tab_1_entries;

  /* It's the main terminal screen */
  ScreenInteractive* main_screen;

  /* This will provide the captured output from the buffer*/
  std::wstring captured_output;

 public:
  beagle_window();
  Component get_menu();
  ScreenInteractive* get_screen();
  void execute();
  std::string manage_command(const char* cmd);
};

}  // namespace ui
