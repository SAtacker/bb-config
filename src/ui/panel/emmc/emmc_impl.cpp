#include <pwd.h>
#include <unistd.h>
#include <filesystem>
#include <fstream>
#include "ftxui/dom/elements.hpp"
#include "ui/panel/panel.hpp"

using namespace ftxui;

namespace ui {

class EMMCImpl : public PanelBase {
 public:
  EMMCImpl() {
    Add(Renderer([&] {
      Elements sizeList;
      updateBlocks();

      for (size_t i = 0; i < blocks.size(); i++) {
        auto free_space = std::to_wstring(
            std::filesystem::space("/dev/" + to_string(blocks[i])).free);
        auto cap = std::to_wstring(
            std::filesystem::space("/dev/" + to_string(blocks[i])).capacity);
        auto avail = std::to_wstring(
            std::filesystem::space("/dev/" + to_string(blocks[i])).available);
        sizeList.push_back(hbox({
                               text(L" Free: " + free_space) | flex,
                               text(L" Cap: " + cap) | flex,
                               text(L" Avail:" + avail) | flex,
                           }) |
                           border);
      }
      auto currentUserPath = std::string(getpwuid(geteuid())->pw_dir);
      auto free_space =
          std::to_wstring(std::filesystem::space(currentUserPath).free);
      auto cap =
          std::to_wstring(std::filesystem::space(currentUserPath).capacity);
      auto avail =
          std::to_wstring(std::filesystem::space(currentUserPath).available);
      sizeList.push_back(hbox({
                             text(to_wstring(currentUserPath)) | flex,
                             text(L" Free: " + free_space) | flex,
                             text(L" Cap: " + cap) | flex,
                             text(L" Avail:" + avail) | flex,
                         }) |
                         border);
      return vbox(sizeList);
    }));
  }
  ~EMMCImpl() = default;
  std::wstring Title() override { return L"EMMC and MicroSD stats"; }

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
};

namespace panel {
Panel EMMC() {
  return Make<EMMCImpl>();
}

}  // namespace panel
}  // namespace ui
