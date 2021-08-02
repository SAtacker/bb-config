#include "ui.hpp"
#include <iostream>
#include <thread>
#include <vector>
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/screen/string.hpp"
#include "ui/focusable.hpp"
#include "ui/panel/panel.hpp"

using namespace ftxui;

namespace ui {
namespace {

// Display a panel with a focusable title on top.
class PanelAdapter : public ComponentBase {
 public:
  PanelAdapter(Panel panel) : panel_(panel) {
    title_ = Make<Focusable>(panel->Title());

    Add(Container::Vertical({
        title_,
        panel_,
    }));
  }

  Element Render() final {
    return vbox({
               title_->Render(),
               separator(),
               vbox({panel_->Render(), filler()}),
           }) |
           flex;
  }

 private:
  Component title_;
  Panel panel_;
};

// Add a line with a give |title| on top of a component.
Component Header(std::wstring title, Component body) {
  // Add extra margin on the left.
  title = L"─" + title;

  return Renderer(body, [=] {
    return vbox({
        dbox({
            separator(),
            text(title),
        }),
        body->Render(),
    });
  });
}

struct Group {
  std::wstring title;
  std::vector<Panel> groups;
};

class MainMenu : public ComponentBase {
 public:
  MainMenu(std::vector<Group> menu_group, ScreenInteractive* screen) {
    // The sub menu:
    index_.resize(menu_group.size());
    menu_entries_.resize(menu_group.size());
    menu_.resize(menu_group.size());
    tab_.resize(menu_group.size());

    MenuOption menu_option;
    menu_option.style_selected =
        bold | color(Color::Chartreuse1) | bgcolor(Color::Black);
    menu_option.style_focused =
        color(Color::Black) | bgcolor(Color::Chartreuse1);
    menu_option.style_selected_focused =
        bold | color(Color::Black) | bgcolor(Color::Chartreuse1);

    for (size_t i = 0; i < menu_group.size(); ++i) {
      menu_[i] = Menu(&menu_entries_[i], &index_[i], menu_option);
      tab_[i] = Container::Tab({}, &index_[i]);
      index_[i] = 0;
      for (auto it : menu_group[i].groups) {
        menu_entries_[i].push_back(it->Title());
        tab_[i]->Add(Make<PanelAdapter>(std::move(it)));
      }

      menu_[i] = Header(menu_group[i].title, menu_[i]);
    }

    // The global menu.
    group_menu_ = Container::Vertical(menu_, &group_index_);
    group_tab_ = Container::Tab(tab_, &group_index_);

    ButtonOption exitButtonOption;
    exitButtonOption.border = false;
    exit_button_ = Button(L"Exit", screen->ExitLoopClosure(), exitButtonOption);

    Add(Container::Vertical({
        exit_button_,
        Container::Horizontal({
            Container::Vertical({
                group_menu_,
            }),
            group_tab_,
        }),
    }));
  }

  Element Render() override {
    iteration_++;
    auto title =
        text(L" beagle-config ") | bold | color(Color::Cyan1) | hcenter;
    return window(
               title,
               hbox({
                   vbox({
                       hbox({
                           spinner(5, iteration_),
                           text(L"  beagle-config"),
                           filler(),
                           exit_button_->Render() | color(Color::DarkOrange3),
                       }),
                       group_menu_->Render() | yframe,
                   }),
                   separator(),
                   group_tab_->Render() | flex,
               })) |
           bgcolor(Color::Black);
  }

 private:
  // The nested menu.
  std::vector<int> index_;
  std::vector<std::vector<std::wstring>> menu_entries_;
  std::vector<Component> menu_;
  std::vector<Component> tab_;

  // The global menu.
  int group_index_ = 0;
  Component group_menu_;
  Component group_tab_;
  Component exit_button_;

  // Allow visualizing when the UI is updated.
  int iteration_ = 0;
};

}  // namespace

void Loop() {
  auto screen = ScreenInteractive::Fullscreen();
  std::vector<Group> groups = {
      {L"System",
       {
           panel::PRU(),
           panel::GPIO(),
           panel::EMMC(),
           panel::Led(),
           panel::PlaceHolder(L"Freeze Packages"),
           panel::PlaceHolder(L"Sensor Stats and Configurations "),
           panel::PlaceHolder(L"Password"),
           panel::PlaceHolder(L"Boot / Auto login "),
           panel::PlaceHolder(L"Firmware Update"),
           panel::PlaceHolder(L"Uboot Overlays"),
           panel::PlaceHolder(L"Overlay FS"),
           panel::PlaceHolder(L"SSH"),
       }},
      {L"Network",
       {
           panel::WiFi(&screen),
           panel::ICS(),
       }},
      {L"Display",
       {
           panel::PlaceHolder(L"Display Resolution"),
       }},
      {L"Info",
       {
           panel::PlaceHolder(L"Update"),
           panel::PlaceHolder(L"About"),
       }},
  };

  Component main_menu = Make<MainMenu>(std::move(groups), &screen);
  screen.Loop(main_menu);
}

}  // namespace ui
