#include "ftxui/component/component.hpp"
#include "ftxui/dom/elements.hpp"
#include "ui/panel/panel.hpp"

#include <vector>
#include <regex>
#include <sstream>
#include <fstream>

#define FILE_PATH "/boot/uEnv.txt"
#define CAPES_REGEX "(###Disable auto loading of virtual capes)(.*)"

using namespace ftxui;

namespace ui {

struct struct_virtual_capes{
    std::string name;
    int file_seekf;
    bool status;
};

class CapesImpl : public PanelBase {
    public:
        CapesImpl() {
            get_caps_status();

            for (std::vector<int>::size_type i = 0; i < _virt_capes.size(); ++i) {
                container->Add(Checkbox(_virt_capes[i].name, &_virt_capes[i].status));
            }

            Add(Container::Vertical({
                                container,
                                _button_update,
                            }));
        }

        ~CapesImpl() = default;

        std::string Title() override { return "Capes"; }

    private:
        std::vector<struct struct_virtual_capes> _virt_capes;
        Component container = Container::Vertical({});
        Component _button_update = Button("Save", [this] { write_cape_status(); });

        void get_caps_status() {
            _virt_capes.clear();

            std::ifstream env_file;
            env_file.open(FILE_PATH);

            if (!env_file)
                std::cout << "Error Openning the File\n";

            std::string file_line;

            while(getline(env_file, file_line)) {
                if (std::regex_match (file_line, std::regex(CAPES_REGEX) )) {
                    struct struct_virtual_capes vc;
                    vc.file_seekf = env_file.tellg();
                    bool flags = false;

                    while(getline(env_file, file_line)) {
                        std::stringstream str_stm(file_line);
                        std::string tmp;

                        if ((std::regex_match (file_line, std::regex("(###)(.*)") )))
                            break;

                        while( getline(str_stm, tmp, '_' )) {
                            ;
                        }

                        std::stringstream str_stm2(tmp);
                        getline(str_stm2, tmp, '=');

                        vc.name = tmp;

                        if (file_line[0] == '#')
                            vc.status = true;
                        else
                            vc.status = false;
                        
                        _virt_capes.push_back(vc);

                        if (!flags) {
                            flags = true;
                            vc.file_seekf = env_file.tellg();
                        }
                        else 
                            vc.file_seekf = env_file.tellg();

                    }
                }
            }

            env_file.close();
        }

        void write_cape_status() {
            std::fstream myfile;
            std::string write_off;
            std::string write_on;

            myfile.open(FILE_PATH, std::ios::in | std::ios::out);

            for (std::vector<int>::size_type i = 0; i < _virt_capes.size(); ++i) {
                write_off = "disable_uboot_overlay_" + _virt_capes[i].name + "=1 ";
                write_on = "#disable_uboot_overlay_" + _virt_capes[i].name + "=1";

                myfile.seekp(_virt_capes[i].file_seekf);

                if (_virt_capes[i].status)
                    myfile << write_on;
                else if(!_virt_capes[i].status)
                    myfile << write_off;
            }
            
            myfile.close();

            get_caps_status();
        }

        Element Render() override {
            return vbox({
                    text(" Virtual Capes : "),
                    hbox(text(" "), container->Render())
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