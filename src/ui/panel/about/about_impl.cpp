#include "environment.hpp"
#include "ftxui/dom/elements.hpp"
#include "ui/panel/panel.hpp"

using namespace ftxui;

namespace ui {

// clang-format off
const std::string logo[] = {
R"(    ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣠⠴⠒⠛⠉⠉⠉⠉⠓⠲⢤⣀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀)",
R"(  ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣠⠴⢯⣅⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⠳⣄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀)",
R"(  ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢠⡾⠉⠀⠀⠀⠀⠱⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⢧⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀)",
R"(  ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⡞⠀⠀⠀⠀⠀⠀⠀⢳⠀⢀⡠⢤⣄⡀⠀⠀⠀⠠⣎⣹⣧⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀)",
R"(  ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣾⠃⠀⠀⠀⠀⠀⠀⠀⠘⡇⠸⣷⣾⣿⠇⠀⠀⠀⠀⠈⠉⠙⠦⣄⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀)",
R"(  ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣼⠃⠀⠀⠀⠀⠀⠀⠀⠀⠀⢹⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠉⠑⠲⢄⡀⠀⠀⠀⠀⠀⠀)",
R"(  ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣼⠇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⣇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣤⠶⠛⠒⣄⠀⠀⠀⠀)",
R"(  ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣸⠏⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠸⡆⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢠⣿⣤⣤⣴⣾⣿⡇⠀⠀⠀)",
R"(  ⠀⠀⠀⠀⠀⠀⠀⠀⠀⢰⡏⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢻⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⠻⣿⣿⣿⣿⠟⡇⠀⠀⠀)",
R"(  ⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⠁⢀⣼⠇⠀⠀⠀)",
R"(  ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠘⣷⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣸⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣀⣤⡴⠛⣟⠀⠀⠀⠀)",
R"(  ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠙⣷⣄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⡴⠃⠀⠀⠀⠀⠀⢀⣤⣤⣤⣶⣶⣾⣿⣿⡋⠉⠀⠀⢸⡆⠀⠀⠀)",
R"(  ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣀⠙⠿⢶⣤⣤⣀⣀⣀⣠⡤⠖⠋⠀⠀⠀⠀⠀⠀⠀⠀⠉⠛⠛⠿⠿⢿⣿⣿⢷⣄⠀⣀⡼⠁⠀⠀⠀)",
R"(  ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣿⠈⢱⠀⠀⢠⡟⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⠁⠀⠐⠒⠒⠂⠈⢯⠉⠉⠀⠈⠉⠛⠉⠁⠀⠀⠀⠀)",
R"(  ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣿⡀⠈⣇⠀⣼⠃⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠸⡄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀)",
R"(  ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠸⣷⡀⠘⢆⣿⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⡀⠀⠀⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀)",
R"(  ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠹⣷⡀⠘⢿⠀⠀⢸⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸⡇⠀⠀⣷⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀)",
R"(  ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠙⣷⡀⠀⠀⠀⢸⡇⠀⠀⠀⡄⠀⠀⠀⠀⠀⣀⠀⠀⠀⠸⡇⠀⠀⡟⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀)",
R"(  ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠘⢳⣄⠀⠀⢸⡇⠀⠀⠀⣿⠀⠀⠀⠀⠀⣿⠀⠀⠀⠀⡇⠀⠀⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀)",
R"(  ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢻⡇⠀⢸⠀⠀⠀⠀⣿⠀⠀⠀⠀⠀⡇⠀⠀⠀⠀⡇⠀⢸⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀)",
R"(  ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣠⠾⠛⠀⢸⠀⠀⠀⠀⢹⠀⠀⠀⠀⠀⡇⠀⠀⠀⠀⣧⠀⠛⠦⡄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀)",
R"(  ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣼⠇⠀⠀⢀⡿⠀⠀⠀⠀⢸⠀⠀⠀⠀⢰⡇⠀⠀⠀⠀⢻⡀⠀⠀⠈⡆⠀⠀⠀⠀⠀⠀⠀⠀⠀)",
R"(              ⠀⢿⣆⡀⠀⡾⠁⠀⠀⠀⠀⢸⡟⠛⠛⠛⣿⡁⠀⠀⠀⠀⠀⢣⠀⢀⣠⠇        ⠀)",
R"(              ⠀⠀⠙⠛⣿⣇⣀⠀⣀⣀⣠⡾⠃⠀⠀⠀⠻⣷⣄⣀⣀⠀⣀⣸⡟⠛⠉⠀        ⠀)",
R"(               ⠀⠀⠀⠉⠛⠻⠿⠛⠛⠉⠀⠀⠀⠀⠀⠀⠈⠙⠛⠻⠿⠛⠋⠀⠀          ⠀)",
R"(  _                      _                             __ _       )",
R"( | |__   ___  __ _  __ _| | ___        ___ ___  _ __  / _(_) __ _ )",
R"( | '_ \ / _ \/ _` |/ _` | |/ _ \_____ / __/ _ \| '_ \| |_| |/ _` |)",
R"( | |_) |  __/ (_| | (_| | |  __/_____| (_| (_) | | | |  _| | (_| |)",
R"( |_.__/ \___|\__,_|\__, |_|\___|      \___\___/|_| |_|_| |_|\__, |)",
R"(                   |___/                                    |___/)",
};
// clang-format on

class AboutImpl : public PanelBase {
 public:
  AboutImpl() {
    opt.border = false;
    button_text = "View Logo";
    viewLogo = Button(
        &button_text,
        [&] {
          view_logo = !view_logo;
          if (view_logo) {
            button_text = "Go Back";
          } else {
            button_text = "View Logo";
          }
        },
        opt);
    Add(viewLogo);
  }
  ~AboutImpl(){};
  std::string Title() override { return "About"; }
  Element RenderBackground() {
    Elements elements;
    for (const std::string& it : logo) {
      elements.push_back(text(it));
    }
    return vbox(std::move(elements)) | center;
  }

  Element RenderForeground() {
    if (view_logo)
      return viewLogo->Render() | hcenter;

    return vbox({
               hbox({
                   vbox({
                       text("Application"),
                       separator(),
                       text("Description"),
                       text(""),
                       text(""),
                       separator(),
                       text("Website"),
                       separator(),
                       text("Commit"),
                       separator(),
                       text("Version"),
                   }) | bold,
                   separator(),
                   vbox({
                       text("beagle-config"),
                       separator(),
                       text("Beagle-Config is a tool-set, that aims to provide "
                            "the"),
                       text("functionality to make the most common low-level"
                            "configuration "),
                       text("changes in beagle devices easily"),
                       separator(),
                       text("https://github.com/SAtacker/beagle-config   "),
                       separator(),
                       text(
                           "https://github.com/SAtacker/beagle-config/commit/" +
                           std::string(git_hash).substr(10) + "   "),
                       separator(),
                       text(git_version),
                   }),
               }),
               separator(),
               viewLogo->Render() | hcenter,
           }) |
           border | hcenter | bgcolor(Color::Black) | clear_under;
  }

  Element Render() override {
    return dbox({
        vbox({
            filler(),
            filler(),
            RenderBackground() | hcenter,
        }),
        vbox({
            RenderForeground(),
        }),
    });
  }

 private:
  std::string title_;
  bool view_logo = false;
  std::string button_text;
  ButtonOption opt;
  Component viewLogo;
};

namespace panel {
Panel About() {
  return Make<AboutImpl>();
}

}  // namespace panel
}  // namespace ui
