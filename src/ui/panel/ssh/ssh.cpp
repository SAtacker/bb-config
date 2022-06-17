#include "ftxui/component/component.hpp"
#include "ui/panel/panel.hpp"
#include "utils.hpp"

using namespace ftxui;

namespace ui {

namespace {

class sshImpl : public PanelBase {
 public:
  sshImpl() {
    get_ssh_status();
    enable = Button("Enable", [&] {
      shell_helper("ssh-keygen -A");
      shell_helper("update-rc.d ssh enable");
      shell_helper("invoke-rc.d ssh start");
      get_ssh_status();
    });
    disable = Button("Disable", [&] {
      shell_helper("update-rc.d ssh disable");
      shell_helper("invoke-rc.d ssh stop");
      get_ssh_status();
    });
    Component page = Renderer(Container::Vertical({
                                  Container::Horizontal({
                                      enable,
                                      disable,
                                  }),
                              }),
                              [&] {
                                return vbox({
                                    text("Status: " + status),
                                    hbox({
                                        enable->Render() | flex,
                                        disable->Render() | flex,
                                    }),
                                });
                              });
    Add(page);
  }
  ~sshImpl() = default;
  std::string Title() override { return "SSH"; }

 private:
  void get_ssh_status() {
    shell_helper("systemctl status sshd | grep active", &status);
  }
  Component tog;
  Component enable;
  Component disable;
  std::string status;
};

}  // namespace

namespace panel {
Panel ssh() {
  return Make<sshImpl>();
}

}  // namespace panel

}  // namespace ui