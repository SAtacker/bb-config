#include "ftxui/component/component.hpp"
#include "ftxui/dom/elements.hpp"
#include "ui/panel/panel.hpp"

#include <vector>
#include <regex>
#include <sstream>
#include <fstream>

#define FILE_PATH "/boot/uEnv.txt"
#define BACKUP "/boot/uEnv_bkp.txt"

using namespace ftxui;

namespace ui {

struct Overlay_Option {
    std::string name;
    int file_seekf;
    bool status;
};

struct Overlays {
    std::string name;
    std::vector<Overlay_Option> *option;
};


class CapesImpl : public PanelBase {
    public:
        CapesImpl() {
            write_cape_status();
            
            for (std::vector<int>::size_type i = 0; i < vect_overlay_.size(); ++i) {
                for (std::vector<int>::size_type j = 0; i < vect_overlay_.at(i).option->size(); ++j) {
                    container_option->Add(Checkbox(
                        vect_overlay_.at(i).option->at(j).name, 
                        &vect_overlay_.at(i).option->at(j).status
                    ));
                }
                container_overlays->Add(container_option);
            }

            Add(Container::Vertical({
                                container_overlays,
                                _button_update,
                            }));
        }

        ~CapesImpl() = default;

        std::string Title() override { return "Capes"; }

    private:
        std::vector<Overlays> vect_overlay_;
        Component container_option = Container::Vertical({});
        Component container_overlays = Container::Vertical({});
        Component _button_update = Button("Save", [this] { write_cape_status(); });

        void get_overlay_status() {
            vect_overlay_.clear();

            std::ifstream inFile;
            inFile.open(FILE_PATH);

            if (!inFile)
                std::cout << "Error Openning the File\n";

            std::string file_line;
            bool s_flags = false; //Flag for Starting and Ending
            bool overlay_flags = false; //Flag for Sub-Overlay
            Overlays *ptr_overlay;
            Overlay_Option *ptr_option;

            while(getline(inFile, file_line)) {
                if (!file_line.compare("###U-Boot Overlays###") && !s_flags)
                {
                    s_flags = true;
                }
                else if (!file_line.compare("###U-Boot Overlays###") && s_flags)
                {
                    vect_overlay_.push_back(*ptr_overlay);
                    break;
                }

                if (file_line.compare("###") == 0) {
                    vect_overlay_.push_back(*ptr_overlay);
                    overlay_flags = false;
                }

                if (overlay_flags) {
                    if (std::regex_match(file_line, std::regex("(###[a-z])(.*)"))) {
                        continue;
                    }
                    std::string temp = file_line;
                    temp.erase(remove(temp.begin(), temp.end(), '#'), temp.end());
                    
                    ptr_option->name = temp;
                    if (file_line[0] == '#') {
                        ptr_option->status = true;
                    }
                    ptr_overlay->option->push_back(*ptr_option);
                    ptr_option->file_seekf = inFile.tellg();
                }

                if (std::regex_match(file_line, std::regex("(###[A-Z])(.*)"))) {
                    if (std::regex_match(file_line, std::regex("(###Documentation)(.*)")))
                        continue;
                    else if (std::regex_match(file_line, std::regex("(###U-Boot Overlays###)")))
                        continue;
                    
                    std::string temp = file_line;
                    temp.erase(remove(temp.begin(), temp.end(), '#'), temp.end());

                    overlay_flags = true;
                    ptr_overlay = new Overlays;
                    ptr_overlay->option = new std::vector<Overlay_Option>;
                    ptr_option = new Overlay_Option;
                    ptr_overlay->name = temp;
                    ptr_option->file_seekf = inFile.tellg();
                }
            }

            inFile.close();
        }

        void write_cape_status() {
            std::ofstream outfile;
            std::ifstream infile;
            std::string in_line;

            infile.open(FILE_PATH);
            outfile.open(BACKUP);

            for (std::vector<int>::size_type i = 0; i < vect_overlay_.size(); ++i) {
                for (std::vector<int>::size_type j = 0; i < vect_overlay_.at(i).option->size(); ++j) {
                    while(getline(infile, in_line)) {
                        if (outfile.tellp() == vect_overlay_.at(i).option->at(j).file_seekf) {
                            if (vect_overlay_.at(i).option->at(j).status)
                                outfile << "#" 
                                        << vect_overlay_.at(i).option->at(j).name 
                                        << std::endl;
                            else if(!vect_overlay_.at(i).option->at(j).status)
                                outfile << vect_overlay_.at(i).option->at(j).name 
                                        << std::endl;
                        }
                        else {
                            outfile << in_line << std::endl;
                        }
                    }
                }
            }
            
            infile.close();
            outfile.close();

            get_overlay_status();
        }

        Element Render() override {
            return vbox({
                    text(" Virtual Capes : "),
                    hbox(text(" "), container_overlays->Render())
                         | vscroll_indicator | frame |size(HEIGHT, LESS_THAN, 10),
                    separator(),
                    _button_update->Render(),
                });
        }
};

namespace panel {
Panel Capes() {
  return Make<CapesImpl>();
}

}  // namespace panel

}  // namespace ui