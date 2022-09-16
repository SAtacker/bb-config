#include "ftxui/component/component.hpp"
#include "ftxui/dom/elements.hpp"
#include "ui/panel/panel.hpp"
#include "utils.hpp"

#include "board_image.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

using namespace ftxui;

namespace ui {

namespace {

const std::string PinConfigPath = "/sys/devices/platform/ocp";
const std::string DeviceNamePath = "/proc/device-tree/model";

const std::string PocketName = "TI AM335x PocketBeagle";
const std::string BlackName = "TI AM335x BeagleBone Black";
const std::string BlueName = "TI AM335x BeagleBone Blue";
const std::string AI_Name = "BeagleBoard.org BeagleBone AI";

struct PinDetail {
  std::string header;        // eg. P9_22
  std::string name;          // eg. gpio
  std::string info;          // Pin info
  std::string pru;           // Pru pin
  std::string gpio;          // gpio pin
  std::string pinmux;        // List of Pinmux which can config
  std::string pinmux_value;  // Pinmux Current mode
  bool config = false;       // Support PINMUX
};

class ConfigPinMux : public ComponentBase {
 public:
  ConfigPinMux(struct PinDetail pin) : pin_(pin) {
    update_configDropdown();

    Add(Container::Vertical({
        configDropdwon_,
        button_,
    }));
  }

  ~ConfigPinMux() = default;

 private:
  Element Render() override {
    return vbox({
               text(pin_.header),
               text(PinConfigPath + "/ocp:" + pin_.header + "_pinmux/state"),
               separator(),
               configDropdwon_->Render() | flex,
               separator(),
               button_->Render(),
           }) |
           border | flex;
  }

  void update_configDropdown() {
    configValues.clear();
    std::stringstream ss(pin_.pinmux);
    std::string str;

    while (ss.good()) {
      getline(ss, str, ' ');
      configValues.push_back(str);
    }
  }

  void config_apply() {
    std::ofstream file(PinConfigPath + "/ocp:" + pin_.header + "_pinmux/state");
    file << configValues[configSelected_];
    file.close();
  }

  struct PinDetail pin_;
  int configSelected_ = 0;
  std::vector<std::string> configValues;
  Component configDropdwon_ = Dropdown(&configValues, &configSelected_);
  Component button_ = Button("Apply", [this] { config_apply(); });
};

class PinMuxImpl : public PanelBase {
 public:
  PinMuxImpl() {
    get_device_name();
    get_pin_detail();
    build_hardware_tab();
    build_menu_tab();
    get_pinmux_status();
    build_pinmux_tab();

    Add(Container::Vertical({
        tabMenu_,
        tabContent_,
    }));
  }

  ~PinMuxImpl() = default;

  std::string Title() override { return "PinMux"; }

 private:
  // Get device name
  void get_device_name() { std::ifstream(DeviceNamePath) >> device_name_; }

  // Get pin detail from file
  void get_pin_detail() {
    std::ifstream inFile;
    std::string pin_header_selection;
    if (!device_name_.compare(PocketName)) {
      inFile.open("../src/ui/panel/pinmux/pin_info_pocket");
      pin_header_selection = "P1";
    } else {
      inFile.open("../src/ui/panel/pinmux/pin_info");
      pin_header_selection = "P8";
    }

    std::string file_line;
    pin_P1_P8_.clear();
    pin_P2_P9_.clear();

    while (getline(inFile, file_line)) {
      if (file_line.compare("") == 0)
        continue;

      std::string head, tail, info, func, last;

      struct PinDetail temp_pin;

      do {
        std::stringstream str(file_line);
        getline(str, head, '_');
        getline(str, tail, '_');
        getline(str, last, '_');
        std::stringstream str2(last);
        getline(str2, info, '=');
        std::stringstream str3(file_line);
        while (str3.good()) {
          getline(str3, func, '=');
        }

        temp_pin.header = head + "_" + tail;
        func = func.substr(1, func.size() - 2);
        if (!info.compare("INFO")) {
          temp_pin.info = func;
        } else if (!info.compare("PIN")) {
          temp_pin.name = func;
        } else if (!info.compare("PRU")) {
          temp_pin.pru = func;
        } else if (!info.compare("GPIO")) {
          temp_pin.gpio = func;
        } else if (!info.compare("PINMUX")) {
          temp_pin.pinmux = func;
        } else if (!info.compare("CAPE")) {
          if (!head.compare(pin_header_selection)) {
            pin_P1_P8_.push_back(temp_pin);
          } else {
            pin_P2_P9_.push_back(temp_pin);
          }

          break;
        }
      } while (getline(inFile, file_line));
    }

    inFile.close();
  }

  // Get pinmux info current state
  void get_pinmux_status() {
    const std::filesystem::path p = PinConfigPath;
    if (!std::filesystem::exists(p)) {
      config_features = false;
      return;
    }

    for (const auto& it : std::filesystem::directory_iterator(PinConfigPath)) {
      std::string path = it.path();
      std::stringstream ss(path);
      std::string name;

      while (ss.good()) {
        getline(ss, name, '/');
      }

      std::stringstream ss3(name);
      getline(ss3, name, ':');

      if (name.compare("ocp"))
        continue;

      getline(ss3, name, ':');
      std::stringstream ss2(name);

      std::string head, pin;
      getline(ss2, head, '_');
      getline(ss2, pin, '_');
      getline(ss2, name, '_');

      std::string pinmux_value;
      std::string pin_header_selection;
      std::ifstream(path + "/status") >> pinmux_value;

      if (!device_name_.compare(PocketName)) {
        pin_header_selection = "P1";
      } else {
        pin_header_selection = "P8";
      }

      std::stringstream ss4(pin);
      int pin_num;

      ss4 >> pin_num;

      if (pin_num - 1 >= 46)
        continue;

      if (!name.compare("pinmux")) {
        if (!head.compare(pin_header_selection)) {
          pin_P1_P8_[pin_num - 1].config = true;
          pin_P1_P8_[pin_num - 1].pinmux_value = pinmux_value;
        } else {
          pin_P2_P9_[pin_num - 1].config = true;
          pin_P2_P9_[pin_num - 1].pinmux_value = pinmux_value;
        }
      }
    }
  };

  // Display Hardware
  void build_hardware_tab() {
    Component pageHardware = Renderer([&] { return renderHardware(); });
    tabContent_->Add(pageHardware);
  }

  // Display pin menu
  void build_menu_tab() {
    for (int i = 0; i < (int)pin_P1_P8_.size(); i++) {
      ftxui::Color c = ColorCode[convert_colorCode(pin_P1_P8_[i].name)];
      if (i % 2 == 0) {
        menu_P1P8_left_->Add(MenuEntry(pin_P1_P8_[i].name, Colored(c)));
      } else {
        menu_P1P8_right_->Add(MenuEntry(pin_P1_P8_[i].name, Colored(c)));
      }
    }

    for (int i = 0; i < (int)pin_P2_P9_.size(); i++) {
      ftxui::Color c = ColorCode[convert_colorCode(pin_P2_P9_[i].name)];
      if (i % 2 == 0) {
        menu_P2P9_left_->Add(MenuEntry(pin_P2_P9_[i].name, Colored(c)));
      } else {
        menu_P2P9_right_->Add(MenuEntry(pin_P2_P9_[i].name, Colored(c)));
      }
    }

    tabContent_->Add(PinDetail());
  }

  // Display Config pinmux
  void build_pinmux_tab() {
    if (config_features)
      tabContent_->Add(PinConfig());
    else
      tabContent_->Add(Renderer([&] { return featuresNotSupport(); }));
  }

  // Create color for Menu
  MenuEntryOption Colored(ftxui::Color c) {
    MenuEntryOption option;
    option.transform = [](EntryState state) {
      state.label = (state.focused ? "> " : "  ") + state.label + " ";
      Element e = text(state.label) | hcenter;
      if (state.focused)
        e = e | inverted;
      if (state.focused)
        e = e | bold;
      return e;
    };

    option.animated_colors.background.enabled = true;
    option.animated_colors.background.inactive = c;

    return option;
  }

  // Render PinMenu Tab
  Component PinDetail() {
    Component page = Renderer(
        Container::Horizontal(
            {
                menu_P1P8_left_,
                menu_P1P8_right_,
                menu_P2P9_left_,
                menu_P2P9_right_,
            },
            &header_menu_selected_),
        [&] {
          Elements num_8l, num_8r, num_9l, num_9r, content;
          int num_pins = (device_name_.compare(PocketName)) ? 46 : 36;

          for (int i = 1; i <= num_pins; i++) {
            if (i % 2 != 0)
              num_8l.push_back(text(std::to_string(i)));
            else
              num_8r.push_back(text(std::to_string(i)));
          }

          for (int i = 1; i <= num_pins; i++) {
            if (i % 2 != 0)
              num_9l.push_back(text(std::to_string(i)));
            else
              num_9r.push_back(text(std::to_string(i)));
          }

          int g = header_menu_selected_;

          if (g < 2) {
            content.push_back(text(
                "Content : " + pin_P1_P8_[head_selected_[g] * 2 + g].header));
            content.push_back(
                text("Name : " + pin_P1_P8_[head_selected_[g] * 2 + g].name));
          } else {
            content.push_back(
                text("Content : " +
                     pin_P2_P9_[head_selected_[g] * 2 + (g - 2)].header));
            content.push_back(text(
                "Name : " + pin_P2_P9_[head_selected_[g] * 2 + (g - 2)].name));
          }
          return vbox({
                     hbox({
                         menu_P1P8_left_->Render() |
                             size(WIDTH, GREATER_THAN, 10),
                         separator(),
                         vbox(num_8l),
                         separator(),
                         vbox(num_8r),
                         separator(),
                         menu_P1P8_right_->Render() |
                             size(WIDTH, GREATER_THAN, 10),
                         separator(),
                         vbox({
                             content,
                         }) | flex,
                         separator(),
                         menu_P2P9_left_->Render() |
                             size(WIDTH, GREATER_THAN, 10),
                         separator(),
                         vbox(num_9l),
                         separator(),
                         vbox(num_9r),
                         separator(),
                         menu_P2P9_right_->Render() |
                             size(WIDTH, GREATER_THAN, 10),
                     }) | border,
                 }) |
                 frame | vscroll_indicator;
        });
    return page;
  };

  // Render PinMiux Config Tab
  Component PinConfig() {
    for (auto& pin : pin_P1_P8_) {
      if (pin.config) {
        auto pin_ptr = std::make_shared<ConfigPinMux>(pin);
        menu_content_->Add(pin_ptr);
        config_p8_->Add(MenuEntry(pin.header));
        num_p8++;
      }
    }

    for (auto& pin : pin_P2_P9_) {
      if (pin.config) {
        auto pin_ptr = std::make_shared<ConfigPinMux>(pin);
        menu_content_->Add(pin_ptr);
        config_p9_->Add(MenuEntry(pin.header));
      }
    }

    Component page = Renderer(
        Container::Horizontal({
            Container::Horizontal(
                {
                    config_p8_,
                    config_p9_,
                },
                &config_menu_selected_),
            menu_content_,
        }),
        [&] {
          config_tab_selected_ = (config_menu_selected_ ? num_p8 : 0) +
                                 config_selected_[config_menu_selected_];
          return hbox({
                     config_p8_->Render() | size(WIDTH, GREATER_THAN, 10) |
                         frame | vscroll_indicator,
                     separator(),
                     config_p9_->Render() | size(WIDTH, GREATER_THAN, 10) |
                         frame | vscroll_indicator,
                     separator(),
                     menu_content_->Render(),
                 }) |
                 border;
        });

    return page;
  };

  // Render Pocket Hardware
  Element drawBBBlack() {
    bool h_flag = false, p_flag = false, z_flag = false, s_flag = false,
         i_flag = false;
    int skg = 0, i = 0;
    Elements elements;
    for (const std::string& it : board) {
      Elements column;
      z_flag = false;
      s_flag = true;
      ftxui::Color c;
      for (const char& ch : it) {
        std::string temp;
        temp.push_back(ch);

        if (h_flag) {
          if (++skg == 2) {
            skg = 0;
            h_flag = false;
          }
          column.push_back(text(temp) | bgcolor(Color::Blue));
          continue;
        } else if (p_flag) {
          if (++skg == 5) {
            skg = 0;
            p_flag = false;
            z_flag = true;
          }

          if (s_flag) {
            i++;
            s_flag = false;
          }

          if (ch == '|') {
            column.push_back(text(temp));
            if (z_flag) {
              c = ColorCode[convert_colorCode(pin_P1_P8_[i * 2 - 1].name)];
            } else {
              c = ColorCode[convert_colorCode(pin_P2_P9_[i * 2 - 1].name)];
            }
            continue;
          }

          if (i_flag) {
            i_flag = false;
            if (z_flag) {
              c = ColorCode[convert_colorCode(pin_P1_P8_[i * 2 - 2].name)];
            } else {
              c = ColorCode[convert_colorCode(pin_P2_P9_[i * 2 - 2].name)];
            }
          }

          column.push_back(text(temp) | bgcolor(c));
          continue;
        }

        if (ch == '!') {
          h_flag = true;
          column.push_back(text(" "));
          continue;
        } else if (ch == '#') {
          p_flag = true;
          i_flag = true;
          column.push_back(text(" "));
          continue;
        }

        column.push_back(text(temp));
      }
      elements.push_back(hbox(std::move(column)));
    }
    return vbox(std::move(elements)) | center;
  }

  // Render Black Hardware
  Element drawBBPocket() {
    int i = 0, j = 0;
    Elements elements;
    ftxui::Color c;
    for (const std::string& it : board_pocket) {
      Elements column;
      i++;
      j = 18;
      for (const char& ch : it) {
        std::string str;
        str.push_back(ch);
        if (ch == '#') {
          j--;
          if (i == 3) {
            c = ColorCode[convert_colorCode(pin_P1_P8_[j].name)];
          } else if (i == 4) {
            c = ColorCode[convert_colorCode(pin_P1_P8_[(2 * j) + 1].name)];
          } else if (i == 15) {
            c = ColorCode[convert_colorCode(pin_P2_P9_[j].name)];
          } else if (i == 16) {
            c = ColorCode[convert_colorCode(pin_P2_P9_[(2 * j) + 1].name)];
          }

          column.push_back(text(str) | bgcolor(c));
        } else {
          column.push_back(text(str));
        }
      }
      elements.push_back(hbox(std::move(column)));
    }
    return vbox(std::move(elements)) | center;
  }

  // Render Error Page
  Element featuresNotSupport() {
    return vbox({
        text("Features currently not supoort.") | bold,
    });
  }

  // Render Hardware Tab
  Element renderHardware() {
    if (!device_name_.compare(PocketName))
      return drawBBPocket();
    else if (!device_name_.compare(BlueName))
      return featuresNotSupport();
    else
      return drawBBBlack();
  }

  Element Render() override {
    return vbox({
        tabMenu_->Render(),
        tabContent_->Render(),
    });
  }

  // Assign ColorCode
  int convert_colorCode(const std::string pin) {
    if (!pin.compare("gnd")) {
      return 0;
    } else if (!pin.compare("power")) {
      return 1;
    } else if (!pin.compare("system")) {
      return 1;
    } else if (!pin.compare("gpio")) {
      return 2;
    } else if (!pin.compare("gpio_input")) {
      return 2;
    } else if (!pin.compare("i2c")) {
      return 4;
    } else if (!pin.compare("emmc")) {
      return 5;
    } else if (!pin.compare("hdmi")) {
      return 5;
    } else if (!pin.compare("audio")) {
      return 5;
    } else if (!pin.compare("adc")) {
      return 6;
    } else if (!pin.compare("uart")) {
      return 7;
    } else if (!pin.compare("pruin")) {
      return 8;
    } else {
      return 5;
    }
    return 0;
  }

  bool config_features = true;
  int num_p8 = 0;
  int tab_selected_ = 0;
  int header_menu_selected_ = 0;
  int config_menu_selected_ = 0;
  int config_tab_selected_ = 0;
  int config_selected_[2] = {0, 0};
  int head_selected_[4] = {0, 0, 0, 0};
  std::string device_name_;
  std::vector<struct PinDetail> pin_P1_P8_;
  std::vector<struct PinDetail> pin_P2_P9_;
  std::vector<std::string> tab_names_ = {"Hardware", "Pin Detail", "PINMUX"};
  Component tabMenu_ =
      Menu(&tab_names_, &tab_selected_, MenuOption::HorizontalAnimated());
  Component tabContent_ = Container::Tab({}, &tab_selected_);
  Component menu_P1P8_left_ = Container::Vertical({}, &head_selected_[0]);
  Component menu_P1P8_right_ = Container::Vertical({}, &head_selected_[1]);
  Component menu_P2P9_left_ = Container::Vertical({}, &head_selected_[2]);
  Component menu_P2P9_right_ = Container::Vertical({}, &head_selected_[3]);
  Component config_p8_ = Container::Vertical({}, &config_selected_[0]);
  Component config_p9_ = Container::Vertical({}, &config_selected_[1]);
  Component menu_content_ = Container::Tab({}, &config_tab_selected_);
};

}  // namespace

namespace panel {
Panel PinMux() {
  return Make<PinMuxImpl>();
}

}  // namespace panel

}  // namespace ui