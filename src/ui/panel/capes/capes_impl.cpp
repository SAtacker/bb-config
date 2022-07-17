#include "ftxui/component/component.hpp"
#include "ftxui/dom/elements.hpp"
#include "ui/panel/panel.hpp"

#include <vector>
#include <regex>
#include <sstream>
#include <fstream>

#define FILE_PATH "/boot/uEnv.txt"

using namespace ftxui;

namespace ui {

struct Option {
    std::string name;
    int fseek;
    bool status;
};

class Overlay : public ComponentBase {
public:
    Overlay(std::string name) : name_(name) {
        options_.clear();
        trigger_selected_ = 0;

        fetchState();

        for (auto option : options_) {
            container_->Add(Checkbox(option.name, &option.status));
        }

        Component page = Renderer(Container::Vertical({
                                  container_,
                                  trigger_
                              }),
                              [&] {
                                return vbox({
                                    text(name_) | bold,
                                    container_->Render() | vscroll_indicator | flex,
                                    trigger_->Render(),
                                    separator(),
                                });
                              });

        Add(page);
    }

private:
    int trigger_selected_ = 0;
    std::string name_;
    std::vector<struct Option> options_;
    Component container_ = Container::Vertical({});
    Component trigger_ = Button("Trigger", [this] {});

    void fetchState() {
        options_.clear();
        std::ifstream inFile;
        inFile.open(FILE_PATH);

        std::string file_line;
        bool c_flags = false;

        Option option;

        while(getline(inFile, file_line)) {
            if (std::regex_match(file_line, std::regex("(###[A-Z])(.*)"))) {
                std::string temp = file_line;
                temp.erase(remove(temp.begin(), temp.end(), '#'), temp.end());

                if (name_.compare(temp) == 0) {
                    c_flags = true;
                    option.fseek = inFile.tellg();
                    continue;
                }
            }

            if (c_flags) {
                if (file_line.compare("###") == 0) 
                    break;
                if (std::regex_match(file_line, std::regex("(###U-Boot Overlays###)")))
                    break;
                if (std::regex_match(file_line, std::regex("(###[a-z])(.*)"))) 
                    continue;
                
                std::string temp = file_line;
                temp.erase(remove(temp.begin(), temp.end(), '#'), temp.end());

                option.name = temp;
                if (file_line[0] == '#') {
                    option.status = true;
                }
                else {
                    option.status = false;
                }

                options_.push_back(option);

                option.fseek = inFile.tellg();
            }
        }

        inFile.close();
    }
};

class CapesImpl : public PanelBase {
    public:
        CapesImpl() {
            fetchStates();

            for (auto overlay : overlays_) {
                container_->Add(Make<Overlay>(overlay));
            }

            Add(Container::Vertical({
                radiobox_,
                container_,
            }));
        }

        ~CapesImpl() = default;

        std::string Title() override { return "Overlays"; }
    private:
        int selected_overlay_ = 0;
        std::vector<std::string> overlays_;

        Component radiobox_ = Radiobox(&overlays_, &selected_overlay_);
        Component container_ = Container::Tab({}, &selected_overlay_);
        Component trigger_ = Button("Trigger", [this] {} );

        void fetchStates() {
            std::fstream inFile;
            inFile.open(FILE_PATH);

            std::string file_line;
            bool c_flags = false;

            if (!inFile)
                std::cout << "Error Openning the File\n";

            while(getline(inFile, file_line)) {
                if (!file_line.compare("###U-Boot Overlays###") && !c_flags)
                {
                    c_flags = true;
                    continue;
                }
                else if (!file_line.compare("###U-Boot Overlays###") && c_flags)
                {
                    break;
                }

                if (std::regex_match(file_line, std::regex("(###[A-Z])(.*)"))) {
                    if (std::regex_match(file_line, std::regex("(###Documentation)(.*)")))
                        continue;
                    else if (std::regex_match(file_line, std::regex("(###U-Boot Overlays###)")))
                        continue;
                    
                    std::string temp = file_line;
                    temp.erase(remove(temp.begin(), temp.end(), '#'), temp.end());

                    overlays_.push_back(temp);

                    continue;
                }
            }

            inFile.close();
        }

        Element Render() override {
            return vbox({
                    text("Select a Overrlay Options : ") | bold,
                    hbox(text(" "), radiobox_->Render()),
                    separator(),
                    container_->Render() | vscroll_indicator | flex,
                });
        }
};

namespace panel {
Panel Capes() {
  return Make<CapesImpl>();
}

}  // namespace panel

}  // namespace ui