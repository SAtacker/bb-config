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
  AboutImpl() {}
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
    return hbox({
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
            text("Beagle-Config is a tool-set, that aims to provide the"),
            text("functionality to make the most common low-level"
                 "configuration "),
            text("changes in beagle devices easily"),
            separator(),
            text("https://github.com/SAtacker/beagle-config   "),
            separator(),
            text("https://github.com/SAtacker/beagle-config/commit/" +
                 std::string(git_hash).substr(10) + "   "),
            separator(),
            text(git_version),
        }),
    });
  }

  Element Render() override {
    return dbox({
        vbox({
            filler(),
            filler(),
            RenderBackground() | hcenter,
        }),
        vbox({
            RenderForeground() | border | hcenter | bgcolor(Color::Black) |
                clear_under,
        }),
    });
  }

 private:
  std::string title_;
};

namespace panel {
Panel About() {
  return Make<AboutImpl>();
}

}  // namespace panel
}  // namespace ui
