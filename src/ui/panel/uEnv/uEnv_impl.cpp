#include "ftxui/component/component.hpp"
#include "ftxui/dom/elements.hpp"
#include "ui/panel/panel.hpp"

#include <vector>
#include <regex>
#include <sstream>
#include <fstream>

#define File_uEnv "/boot/uEnv.txt"
#define File_Backup "./uEnv_bkp.txt"

using namespace ftxui;

namespace ui {

struct Enviroment {
    std::string name;
    bool status;
};

class MenuConfigs : public ComponentBase {
public:
    MenuConfigs(const std::string &name, std::vector<Enviroment> *env) 
            : name_(name), env_(env) {
        for (int i = 0; i < (int) env_->size(); i++) {
            container_->Add(Checkbox(env_->at(i).name, &(env_->at(i).status)));
        }

        Add (Container::Vertical({
            container_
        }));
    }

    ~MenuConfigs() {
        delete env_;
    }

    std::vector<Enviroment> *get_Env() {
        return env_;
    }

    Element Render() override {
        return vbox({
            hbox({
                text(name_) | bold,
            }),
            container_->Render(),
        });
    }

private:
    std::string name_;
    std::vector<Enviroment> *env_;
    Component container_ = Container::Vertical({});
};


class uEnvImpl : public PanelBase {
public:
    uEnvImpl() {
        get_menuConfigs();

        for (auto name : menuNames_) {
            get_menuOptions(name);
        }

        Add(Container::Vertical({
            radiobox_,
            env_tab_,
            button_,
        }));
    }

    std::string Title() override { return "uEnv"; }

private:
    int selected = 0; 
    std::vector<std::string> menuNames_;
    std::vector<std::shared_ptr<MenuConfigs>> menuConfig_;
    std::vector<Enviroment> *env_ptr;
    Component env_individual;
    Component radiobox_ = Radiobox(&menuNames_, &selected);
    Component env_tab_ = Container::Tab({}, &selected);
    Component button_ = Button("Apply", [this] { write_uEnv();} );

    Element Render() override {
        return vbox({
            vbox({
                text("Menu Config") | bold,
                radiobox_->Render() | vscroll_indicator,
            }) | frame,
            separator(),
            env_tab_->Render() | flex,
            button_->Render(),
        }) | vscroll_indicator;
    }

    void get_menuConfigs() {
        menuNames_.clear();

        std::ifstream inFile;
        inFile.open(File_uEnv);

        if (!inFile)
            std::cout << "Error Openning the File\n";

        std::string file_line;
        bool s_flags = false;

        while(getline(inFile, file_line)) {
            if (!file_line.compare("###U-Boot Overlays###") && !s_flags)
            {
                s_flags = true;
            }
            else if (!file_line.compare("###U-Boot Overlays###") && s_flags)
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

                menuNames_.push_back(temp);
            }
        }

        inFile.close();
    }

    void get_menuOptions(const std::string &config) {
        env_ptr = new std::vector<Enviroment>;

        std::ifstream inFile;
        inFile.open(File_uEnv);

        if (!inFile)
            std::cout << "Error Openning the File\n";

        std::string file_line;
        bool s_flags = false;
        Enviroment tempEnv;

        while(getline(inFile, file_line)) {
            if (s_flags) {
                if (!file_line.compare("###U-Boot Overlays###"))
                {
                    auto menu = std::make_shared<MenuConfigs>(config, env_ptr);
                    menuConfig_.push_back(menu);
                    env_tab_->Add(menu);
                    break;
                }
                if (file_line.compare("###") == 0) {
                    auto menu = std::make_shared<MenuConfigs>(config, env_ptr);
                    menuConfig_.push_back(menu);
                    env_tab_->Add(menu);
                    break;
                }
                if (std::regex_match(file_line, std::regex("(###[a-z])(.*)")))
                    continue;
                
                std::string temp = file_line;
                temp.erase(remove(temp.begin(), temp.end(), '#'), temp.end());

                if (file_line[0] == '#') 
                    tempEnv.status = true;
                else
                    tempEnv.status = false;

                tempEnv.name = temp;

                env_ptr->push_back(tempEnv);
            }

            if (std::regex_match(file_line, std::regex("(###[A-Z])(.*)"))) {

                std::string temp = file_line;
                temp.erase(remove(temp.begin(), temp.end(), '#'), temp.end());

                if (!config.compare(temp)) {
                    s_flags = true;
                }
            }
        }

        inFile.close();
    }

    void reset() {
        menuNames_.clear();
        menuConfig_.clear();

        get_menuConfigs();
        for (auto name : menuNames_) {
            get_menuOptions(name);
        }
        selected = 0;
    }

    void write_uEnv() {
        std::ifstream infile;
        std::ofstream outfile;

        infile.open(File_uEnv);
        outfile.open(File_Backup);

        std::string file_line;
        bool s_flags = false;
        int track = 0;
        std::vector<Enviroment> *tempEnv;

        while(getline(infile, file_line)) {
            if (s_flags) {
                if (std::regex_match(file_line, std::regex("(###[a-z])(.*)")))
                    outfile << file_line << std::endl;

                if (file_line.compare("###") == 0)
                    outfile << file_line << std::endl;

                if (std::regex_match(file_line, std::regex("(###[A-Z])(.*)"))) {
                    if (std::regex_match(file_line, std::regex("(###Documentation)(.*)"))) {
                        outfile << file_line << std::endl;
                        continue;
                    }
                    else if (std::regex_match(file_line, std::regex("(###U-Boot Overlays###)"))) {
                        outfile << file_line << std::endl;
                        s_flags = false;
                        continue;
                    }

                    outfile << file_line << std::endl;
                    tempEnv = menuConfig_.at(track++)->get_Env();
                    for (auto env : *tempEnv) {
                        getline(infile, file_line);
                        if (std::regex_match(file_line, std::regex("(###[a-z])(.*)"))) {
                            outfile << file_line << std::endl;
                            getline(infile, file_line);
                        }

                        if (env.status)
                            outfile << "#" << env.name << std::endl;
                        else
                            outfile << env.name << std::endl;
                    }
                }
            }
            else {
                outfile << file_line << std::endl;
            }

            if (!file_line.compare("###U-Boot Overlays###") && !s_flags)
            {
                s_flags = true;
            }
        }

        infile.close();
        outfile.close();

        remove(File_uEnv);
        rename(File_Backup, File_uEnv);

    }
};

namespace panel {
Panel uEnv() {
  return Make<uEnvImpl>();
}

}  // namespace panel

}  // namespace ui
