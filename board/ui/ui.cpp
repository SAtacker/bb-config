#include "ui.hpp"

using namespace ui;
using namespace custom_container_component;

beagle_window::beagle_window() {
  main_screen = new ScreenInteractive(ftxui::ScreenInteractive::Fullscreen());

  tab_1_entries = {
      L"PRU enable/disable",
      L"GPIO configuration",
      L"Internet Sharing and Client Configurations ",
      L"eMMC and MicroSD stats",
      L"Freeze Packages",
      L"Wireless Configurations",
      L"Sensor Stats and Configurations ",
      L"Password",
      L"Boot / Auto login ",
      L"User LED",
      L"Firmware Update",
  };
  tab_2_entries = {
      L"Resolution",
  };
  tab_3_entries = {
      L"SSH", L"Uboot Overlays", L"Overlay FS", L"Update", L"About",
  };
  tab_1_selected = 0;
  tab_2_selected = 0;
  tab_3_selected = 0;
  tab_menu = panel(
      {
          Menu(&tab_1_entries, &tab_1_selected),
          Menu(&tab_2_entries, &tab_2_selected),
          Menu(&tab_3_entries, &tab_3_selected),
      },
      {
          L"System",
          L"Display",
          L"Interfacing",
      },
      0, &tab_selected);

  right_component_selection.resize(3);
  gpio_selection.resize(5);
  emmc_slider = 20;
  right_component_1 = {
      panel(
          {
              panel(
                  {
                      Button(L"PRU On", [] {}),
                      Button(L"PRU Off", [] {}),
                  },
                  {
                      L"",
                      L"",
                  },
                  1),
              Button(L"Upload", [] {}),
          },
          {
              L"PRU",
              L"Program",
          }),
      panel(
          {
              Checkbox(L"Input / Output", &gpio_selection[0].checked),
              Checkbox(L"Input / Output", &gpio_selection[1].checked),
              Checkbox(L"Input / Output", &gpio_selection[2].checked),
              Checkbox(L"Input / Output", &gpio_selection[3].checked),
              Checkbox(L"Input / Output", &gpio_selection[4].checked),
          },
          {
              L"GPIO 1",
              L"GPIO 2",
              L"GPIO 3",
              L"GPIO 4",
              L"GPIO 5",
          }),
      panel(
          {

              panel(
                  {
                      Button(L"Enable Default ICS",
                             [&] { cmd.set_route_add_rem(true); }),
                      Button(L"Revert Routing Table",
                             [&] { cmd.set_route_add_rem(false); }),
                  },
                  {
                      L"",
                      L"",
                  },
                  1),
              panel(
                  {
                      Input(cmd.def_gw_str, L"192.168.6.1"),
                      Input(cmd.dns_1_str, L"8.8.8.8"),
                      Input(cmd.dns_2_str, L"8.8.4.4"),
                  },
                  {
                      L"Routing Gateway",
                      L"Default DNS 1",
                      L"Default DNS 2",
                  }),
          },
          {
              L"Basic ICS",
              L"Customize",
          }),
      panel(
          {
              panel(
                  {
                      Button(L"eMMC", [] {}),
                      Button(L"SDcard", [] {}),
                  },
                  {
                      L"",
                      L"",
                  },
                  1),
              Slider(L"Filled", &emmc_slider, 0, 100, 1),
          },
          {
              L"Button",
              L"EMMC %",
          }),
      panel(
          {
              panel(
                  {
                      Button(L"Freeze", [] {}),
                      Button(L"package", [] {}),
                  },
                  {
                      L"",
                      L"",
                  },
                  1),
          },
          {
              L"Button",
          }),
      panel(
          {
              panel(
                  {
                      Button(L"Wifi On", [] {}),
                      Button(L"Wifi Off", [] {}),
                  },
                  {
                      L"",
                      L"",
                  },
                  1),
          },
          {
              L"Button",
          }),
      panel(
          {
              panel(
                  {
                      Button(L"Sensor 10", [] {}),
                      Button(L"Sensor 20", [] {}),
                  },
                  {
                      L"",
                      L"",
                  },
                  1),
          },
          {
              L"Button",
          }),
      panel(
          {
              panel(
                  {
                      Button(L"Password", [] {}),
                      Button(L"Reset", [] {}),
                  },
                  {
                      L"",
                      L"",
                  },
                  1),
          },
          {
              L"Button",
          }),
      panel(
          {
              panel(
                  {
                      Button(L"Boot", [] {}),
                      Button(L"Auto Login", [] {}),
                  },
                  {
                      L"",
                      L"",
                  },
                  1),
          },
          {
              L"Button",
          }),
      panel(
          {
              panel(
                  {
                      Button(L"UserLED", [] {}),
                      Button(L"USR 1", [] {}),
                  },
                  {
                      L"",
                      L"",
                  },
                  1),
          },
          {
              L"Button",
          }),
      panel(
          {
              panel(
                  {
                      Button(L"Firmware Update", [] {}),
                      Button(L"Update", [] {}),
                  },
                  {
                      L"",
                      L"",
                  },
                  1),
          },
          {
              L"Button",
          }),

  };

  right_component_2 = {
      panel(
          {
              panel(
                  {
                      Button(L"On", [] {}),
                      Button(L"Off", [] {}),
                  },
                  {
                      L"",
                      L"",
                  },
                  1),
          },
          {
              L"Button",
          }),
  };

  right_component_3 = {
      panel(
          {
              panel(
                  {
                      Button(L"On", [] {}),
                      Button(L"Off", [] {}),
                  },
                  {
                      L"",
                      L"",
                  },
                  1),
          },
          {
              L"Button",
          }),
      panel(
          {
              panel(
                  {
                      Button(L"On", [] {}),
                      Button(L"Off", [] {}),
                  },
                  {
                      L"",
                      L"",
                  },
                  1),
          },
          {
              L"Button",
          }),
      panel(
          {
              panel(
                  {
                      Button(L"On", [] {}),
                      Button(L"Off", [] {}),
                  },
                  {
                      L"",
                      L"",
                  },
                  1),
          },
          {
              L"Button",
          }),
      panel(
          {
              panel(
                  {
                      Button(L"On", [] {}),
                      Button(L"Off", [] {}),
                  },
                  {
                      L"",
                      L"",
                  },
                  1),
          },
          {
              L"Button",
          }),
      panel(
          {
              panel(
                  {
                      Button(L"On", [] {}),
                      Button(L"Off", [] {}),
                  },
                  {
                      L"",
                      L"",
                  },
                  1),
          },
          {
              L"Button",
          }),
  };

  tab_selected = 0;
  tab_container = Container::Tab(
      {
          Container::Tab(right_component_1, &tab_1_selected),
          Container::Tab(right_component_2, &tab_2_selected),
          Container::Tab(right_component_3, &tab_3_selected),
      },
      &tab_selected);

  container = Container::Horizontal({
      tab_menu,
      tab_container,
  });

  main_window = Renderer(container, [&] {
    return window(
        text(L"beagle-config") | bold | color(Color::Cyan1) | hcenter,
        hbox(tab_menu->Render() | bgcolor(Color::Black) | color(Color::Cyan1),
             separator(),
             tab_container->Render() | flex | bgcolor(Color::Black)) |
            frame);
  });
}

Component beagle_window::get_menu() {
  return main_window;
}

ScreenInteractive* beagle_window::get_screen() {
  return main_screen;
}

void beagle_window::execute() {
  cmd.ics();
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