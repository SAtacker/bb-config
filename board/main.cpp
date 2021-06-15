#include "cli_parser/cli_parser.hpp"
#include "command_handler/command_handler.hpp"
#include "ui/ui.hpp"

void tui_init() {
  auto main_ = ui::beagle_window();
  auto window = main_.get_menu();
  auto screen = main_.get_screen();
  screen->Loop(window);
}

int main(int argc, char* const argv[]) {
  auto cmd = command_handler::cmd_h();
  auto arg = cli_parser::args_parser();
  arg.add_function([&] { tui_init(); });
  arg.add_function([&](char* optarg, int option) {
    switch (option) {
      case 0:
        strcpy(cmd.def_gw_str, optarg);
        cmd.store_config();
        break;
      case 1:
        strcpy(cmd.dns_1_str, optarg);
        cmd.store_config();
        break;
      case 2:
        strcpy(cmd.dns_2_str, optarg);
        cmd.store_config();
        break;
      case 3:
        cmd.load_config();
        cmd.set_route_add_rem(true);
        break;
      case 4:
        cmd.load_config();
        cmd.set_route_add_rem(false);
        break;
      default:
        break;
    }
  });
  arg.parse(argc, argv);
  return 0;
}
