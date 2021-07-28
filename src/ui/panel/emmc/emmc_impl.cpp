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
std::wstring Format(float value, sizeApprox unit) {
  std::wstringstream ss;
  ss << std::fixed << std::setprecision(2) << (value / float(unit));

  switch (unit) {
    case KB:
      return ss.str() + L" KB";
    case MB:
      return ss.str() + L" MB";
    case GB:
      return ss.str() + L" GB";
    default:
      return L"";
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
  std::wstring Title() override { return L"EMMC and MicroSD stats"; }
  Element Render() override {
    updateBlocks();

    Elements name_list = {text(L"Name"), separator()};
    Elements free_list = {text(L"Free"), separator()};
    Elements capacity_list = {text(L"Capacity"), separator()};
    Elements available_list = {text(L"Start"), separator()};
    Elements gauge_list = {text(L"Filled"), separator()};

    auto add = [&](std::string path) {
      auto space = std::filesystem::space(path);
      name_list.push_back(text(to_wstring(path)));
      free_list.push_back(text(Format(space.free, unit)));
      capacity_list.push_back(text(Format(space.capacity, unit)));
      available_list.push_back(text(Format(space.available, unit)));
      gauge_list.push_back(
          gauge(float(space.capacity - space.free) / float(space.capacity)));
    };

    for (size_t i = 0; i < blocks.size(); i++) {
      if (blocks[i].find(L"loop") != std::string::npos)  // Ignore loop devices.
        continue;

      add("/dev/" + to_string(blocks[i]));
    }

    auto currentUserPath = std::string(getpwuid(geteuid())->pw_dir);
    add(currentUserPath);

    Elements bottom;

    bottom.push_back(hbox({
        resizeButton->Render() | xflex,
        hbox({
            text(L"Show (approx) size in: ") | center,
            kBButton->Render(),
            mBButton->Render(),
            gBButton->Render(),
        }),
    }));

    if (reboot) {
      bottom.push_back(text(L"Reboot to reflect changes"));
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
    }) | yflex;
  }

 private:
  void updateBlocks() {
    std::string path = "/sys/block/";
    blocks.clear();
    for (const auto& entry : std::filesystem::directory_iterator(path)) {
      auto p = std::string(entry.path());
      auto block = p.substr(p.find_last_of("/") + 1);
      blocks.push_back(to_wstring(block));
    }
  }

  std::vector<std::wstring> blocks;
  sizeApprox unit = MB;
  Component kBButton = Button(L"KB", [&] { unit = KB; });
  Component mBButton = Button(L"MB", [&] { unit = MB; });
  Component gBButton = Button(L"GB", [&] { unit = GB; });
  Component resizeButton = Button(L"Grow Partition", [&] {
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
