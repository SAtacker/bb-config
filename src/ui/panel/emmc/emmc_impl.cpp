#include <pwd.h>
#include <unistd.h>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include "ftxui/component/component.hpp"
#include "ftxui/dom/elements.hpp"
#include "ui/panel/panel.hpp"
#include "utils.hpp"

using namespace ftxui;

namespace ui {

namespace {
enum sizeApprox {
  MB = 1024 * 1024,
  GB = 1024 * 1024 * 1024,
  KB = 1024,
};

// Display a |value| as a string with a given |unit| with 2 decimal precision.
std::string Format(float value, sizeApprox unit) {
  std::stringstream ss;
  ss << std::fixed << std::setprecision(2) << (value / float(unit));

  switch (unit) {
    case KB:
      return ss.str() + " KB";
    case MB:
      return ss.str() + " MB";
    case GB:
      return ss.str() + " GB";
    default:
      return "";
  }
}

class EMMCImpl : public PanelBase {
 public:
  EMMCImpl() {
    Add(Container::Horizontal({
        resizeButton,
        kBButton,
        mBButton,
        gBButton,
    })

    );
  }
  ~EMMCImpl() = default;
  std::string Title() override { return "EMMC and MicroSD stats"; }
  Element Render() override {
    updateBlocks();

    Elements name_list = {text("Name"), separator()};
    Elements free_list = {text("Free"), separator()};
    Elements capacity_list = {text("Capacity"), separator()};
    Elements available_list = {text("Start"), separator()};
    Elements gauge_list = {text("Filled"), separator()};

    auto add = [&](std::string path) {
      auto space = std::filesystem::space(path);
      name_list.push_back(text(path));
      free_list.push_back(text(Format(space.free, unit)));
      capacity_list.push_back(text(Format(space.capacity, unit)));
      available_list.push_back(text(Format(space.available, unit)));
      gauge_list.push_back(
          gauge(float(space.capacity - space.free) / float(space.capacity)));
    };

    for (size_t i = 0; i < blocks.size(); i++) {
      if (blocks[i].find("loop") != std::string::npos)  // Ignore loop devices.
        continue;

      add("/dev/" + blocks[i]);
    }

    auto currentUserPath = std::string(getpwuid(geteuid())->pw_dir);
    add(currentUserPath);

    Elements bottom;

    bottom.push_back(hbox({
        resizeButton->Render() | xflex,
        hbox({
            text("Show (approx) size in: ") | center,
            kBButton->Render(),
            mBButton->Render(),
            gBButton->Render(),
        }),
    }));

    if (reboot) {
      bottom.push_back(text("Reboot to reflect changes"));
      reboot = false;
    }

    auto table = hbox({
                     vbox(std::move(name_list)),
                     separator(),
                     vbox(std::move(free_list)),
                     separator(),
                     vbox(std::move(capacity_list)),
                     separator(),
                     vbox(std::move(available_list)),
                     separator(),
                     vbox(std::move(gauge_list)) | xflex,
                 }) |
                 yframe | border;

    return vbox({
               table,
               vbox(std::move(bottom)),
           }) |
           yflex;
  }

 private:
  void updateBlocks() {
    std::string path = "/sys/block/";
    blocks.clear();
    for (const auto& entry : std::filesystem::directory_iterator(path)) {
      auto p = std::string(entry.path());
      auto block = p.substr(p.find_last_of("/") + 1);
      blocks.push_back(block);
    }
  }

  std::vector<std::string> blocks;
  sizeApprox unit = MB;
  Component kBButton = Button("KB", [&] { unit = KB; });
  Component mBButton = Button("MB", [&] { unit = MB; });
  Component gBButton = Button("GB", [&] { unit = GB; });
  Component resizeButton = Button("Grow Partition", [&] {
    shell_helper_no_limit("/opt/scripts/tools/grow_partition.sh");
    reboot = true;
  });
  bool reboot = false;
};
}  // namespace

namespace panel {
Panel EMMC() {
  return Make<EMMCImpl>();
}

}  // namespace panel
}  // namespace ui
