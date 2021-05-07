#include "ui/ui.hpp"

int main(int argc, const char* argv[]) {
  auto screen = ftxui::ScreenInteractive::TerminalOutput();
  int red = 128;
  int green = 25;
  int blue = 100;
  auto component = ui::MyComponent(&red, &green, &blue, screen.ExitLoopClosure());
  screen.Loop(&component);
  std::cout<<"Test"<<std::endl;
  return EXIT_SUCCESS;
}