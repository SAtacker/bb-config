#include "connman_handler.hpp"

int main() {
  connman_handler::connman_h ch;
  ch.data.name = "Name";
  ch.data.pass = "Pass";
  ch.data.type = "wifi";
  std::cout << (ch.wifi_status() ? "Up" : "Down") << "\n";
  std::thread th([&] { ch.connect_wifi(); });
  std::cout << "Current active :: " << ch.get_active_name() << "\n";
  th.join();
  ch.display_service_names();
  for (int i = 0; i < 5; i++) {
    std::cout << (ch.wifi_status() ? "Up" : "Down") << "\n";
    sleep(1);
  }
  ch.disconnect_wifi();
  for (int i = 0; i < 5; i++) {
    std::cout << (ch.wifi_status() ? "Up" : "Down") << "\n";
    sleep(1);
  }
  return 0;
}