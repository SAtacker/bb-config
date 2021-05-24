#include "ui.hpp"

using namespace ui;

int beagle_window::main_selected = 0;
int beagle_window::network_selected = 0;
int beagle_window::tab_index = 0;
int beagle_window::depth = 0;

beagle_window::beagle_window() {
  main_screen = new ScreenInteractive(ftxui::ScreenInteractive::Fullscreen());

  tab_entries = {
      L"System", L"Network",
      // TODO::
      // L"Interfacing",
      // L"Display",
  };

  main_entries = {
      L"sys entry 1",
      L"sys entry 2",
      L"sys entry 3",
  };

  network_entries = {
      L"net entry 1",
      L"net entry 2",
      L"net entry 3",
  };

  main_selected = 0;
  network_selected = 0;
  tab_index = 0;
  captured_output.clear();

  auto main_menu_component = Menu(&main_entries, &main_selected);
  auto network_menu = Menu(&network_entries, &network_selected);

  ftxui::MenuBase::From(main_menu_component)->on_enter = [&] { execute(); };
  ftxui::MenuBase::From(network_menu)->on_enter = [&] { execute(); };

  auto quit_button = Button(L"Quit", main_screen->ExitLoopClosure());
  auto dialog_button_okay = Button("Okay", [&] { depth--; });
  auto help_button = Button(L"Help", main_screen->ExitLoopClosure());

  auto tab_selection = Toggle(&tab_entries, &tab_index);
  auto tab_content = Container::Tab(
      {
          main_menu_component,
          network_menu,
      },
      &tab_index);

  auto dialog_box = Renderer(dialog_button_okay, [this, dialog_button_okay] {
    return window(text(L"Output") | hcenter | bold,
                  vbox(hflow(set_output_window(captured_output)) | yflex,
                       separator(), dialog_button_okay->Render() | hcenter) |
                      flex) |
           color(Color::Cyan1) | bgcolor(Color::Black) | hcenter | flex |
           clear_under;
  });

  auto main_layout = Container::Tab({Container::Vertical({
                                         tab_selection,
                                         tab_content,
                                         Container::Horizontal({
                                             quit_button,
                                             help_button,
                                         }),
                                     }),
                                     dialog_box},
                                    &depth);

  main_menu = Renderer(main_layout, [this, tab_selection, tab_content,
                                     quit_button, help_button, dialog_box] {
    auto document =
        window(text(L"beagle-config") | bold | color(Color::Cyan1) | hcenter,
               vbox(tab_selection->Render() | color(Color::Cyan1) |
                        bgcolor(Color::Black) | hcenter,
                    hbox(tab_content->Render() | flex | color(Color::Cyan1) |
                             bgcolor(Color::Black) | color(Color::Cyan1) |
                             hcenter | flex,
                         hflow(set_output_window(get_help_string()))) |
                        flex,
                    hbox(quit_button->Render() | color(Color::Cyan1) |
                             bgcolor(Color::Black),
                         help_button->Render() | color(Color::Cyan1) |
                             bgcolor(Color::Black)) |
                        hcenter) |
                   bgcolor(Color::Black));
    if (depth == 1) {
      document = dbox({document, dialog_box->Render()});
    }
    return document;
  });
}

Component beagle_window::get_menu() {
  return main_menu;
}

ScreenInteractive* beagle_window::get_screen() {
  return main_screen;
}

int beagle_window::get_selected_number() {
  return main_selected;
}

std::wstring beagle_window::get_selected_option() {
  return main_entries[main_selected];
}

void beagle_window::execute() {
  auto w_option = get_selected_option();
  auto str = manage_command("sh ~/Desktop/gsoc/beagle-config/scripts/intro.sh");
  if (str.at(str.size() - 1) == '\n')
    str.pop_back();
  captured_output = to_wstring(str);
  depth += 1;
}

std::string beagle_window::manage_command(const char* cmd) {
  std::array<char, 128> buffer;
  std::string result;
  std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
  if (!pipe) {
    throw std::runtime_error("popen() failed!");
  }
  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
    result += buffer.data();
  }
  return result;
}

std::wstring beagle_window::get_help_string() {
  if (tab_index == 0)
    return L"This is system help string for " + main_entries[main_selected] +
           L"    Aliquam condimentum elit et finibus euismod. Fusce gravida ";

  return L"This is network string for " + network_entries[network_selected] +
         L"      Sed interdum velit nec massa sollicitudin euismod. Aliquam ";
}

Elements beagle_window::set_output_window(std::wstring str) {
  return paragraph(str);
}
