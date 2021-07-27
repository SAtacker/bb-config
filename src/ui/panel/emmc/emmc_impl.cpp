#include <pwd.h>
#include <unistd.h>
#include <filesystem>
#include <fstream>
#include "ftxui/component/component.hpp"
#include "ftxui/dom/elements.hpp"
#include "ui/panel/panel.hpp"
#include "utils.hpp"

using namespace ftxui;

namespace ui {

enum sizeApprox {
  MB = 1024 * 1024,
  GB = 1024 * 1024 * 1024,
  KB = 1024,
};

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
    Elements elements;
    updateBlocks();
    std::wstring sizeString;
    switch (converter) {
      case KB:
        sizeString = L" KB";
        break;
      case MB:
        sizeString = L" MB";
        break;
      case GB:
        sizeString = L" GB";
        break;
      default:
        sizeString = L" Unknown";
        break;
    }

    elements.push_back(hbox({
                           text(L"*********") | flex,
                           text(L"Free") | flex,
                           text(L"Capacity") | flex,
                           text(L"Available") | flex,
                       }) |
                       border);

    for (size_t i = 0; i < blocks.size(); i++) {
      auto free_space = std::to_wstring(
          std::filesystem::space("/dev/" + to_string(blocks[i])).free /
          (float(converter)));
      auto cap = std::to_wstring(
          std::filesystem::space("/dev/" + to_string(blocks[i])).capacity /
          (float(converter)));
      auto avail = std::to_wstring(
          std::filesystem::space("/dev/" + to_string(blocks[i])).available /
          (float(converter)));
      elements.push_back(hbox({
                             text(blocks[i]) | flex,
                             text(free_space + sizeString) | flex,
                             text(cap + sizeString) | flex,
                             text(avail + sizeString) | flex,
                         }) |
                         border);
    }
    auto currentUserPath = std::string(getpwuid(geteuid())->pw_dir);
    auto free_space = std::to_wstring(
        std::filesystem::space(currentUserPath).free / (float(converter)));
    auto cap = std::to_wstring(
        std::filesystem::space(currentUserPath).capacity / (float(converter)));
    auto avail = std::to_wstring(
        std::filesystem::space(currentUserPath).available / (float(converter)));
    elements.push_back(hbox({
                           text(to_wstring(currentUserPath)) | flex,
                           text(free_space + sizeString) | flex,
                           text(cap + sizeString) | flex,
                           text(avail + sizeString) | flex,
                       }) |
                       border);

    elements.push_back(hbox({
        resizeButton->Render() | flex,
        hbox({text(L"Show (approx) size in: ") | center,
              kBButton->Render() | center, mBButton->Render() | center,
              gBButton->Render() | center}) |
            align_right,
    }));

    if (reboot) {
      elements.push_back(text(L"Reboot to reflect changes"));
      reboot = false;
    }

    return vbox(elements);
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
  sizeApprox converter = MB;
  Component kBButton = Button(L"KB", [&] { converter = KB; });
  Component mBButton = Button(L"MB", [&] { converter = MB; });
  Component gBButton = Button(L"GB", [&] { converter = GB; });
  Component resizeButton = Button(L"Grow Partition", [&] {
    shell_helper_no_limit("/opt/scripts/tools/grow_partition.sh");
    reboot = true;
  });
  bool reboot = false;
};

namespace panel {
Panel EMMC() {
  return Make<EMMCImpl>();
}

}  // namespace panel
}  // namespace ui
