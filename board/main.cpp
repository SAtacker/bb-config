#include "ui/ui.hpp"

int main(int argc, const char* argv[]) {
  auto main_ = ui::beagle_window();
  auto menu = main_.get_menu();
  auto screen = main_.get_screen();
  screen->Loop(menu);
  return 0;
}
