#include <stdio.h>
#include "ftxui/component/component.hpp"
#include "process.hpp"
#include "ui/panel/panel.hpp"
#include "utils.hpp"

using namespace ftxui;

namespace ui {

namespace {

class passwdImpl : public PanelBase {
 public:
  passwdImpl() : user(getlogin()), view(false) {
    InputOption inp_opt;
    inp_opt.password = true;
    inp_opt.cursor_position = 0;
    inp_opt.on_enter = [&] { view = !view; };
    input_p = Input(&pass, "Current Password", inp_opt);
    input_np = Input(&new_pass, "New Password", inp_opt);
    change_ = Button("Change", [&] {
      auto cmd = "passwd " + user;
      FILE* fp = popen(cmd.c_str(), "w");
      fprintf(fp, "%s\n", pass.data());
      fprintf(fp, "%s\n", new_pass.data());
      fprintf(fp, "%s\n", new_pass.data());
      pclose(fp);
    });
    page = Renderer(Container::Vertical({
                        input_p,
                        input_np,
                        change_,
                    }),
                    [&] {
                      if (view) {
                        return vbox({
                            text("User: " + user),
                            text("Pass: " + pass),
                            text("New : " + new_pass),
                            input_p->Render(),
                            input_np->Render(),
                            change_->Render(),
                        });
                      }
                      return vbox({
                          text("User: " + user),
                          input_p->Render(),
                          input_np->Render(),
                          change_->Render(),
                      });
                    });
    Add(page);
  };
  ~passwdImpl() = default;
  std::string Title() override { return "Password"; }

 private:
  Component input_p;
  Component input_np;
  Component change_;
  Component page;
  std::string pass;
  std::string new_pass;
  std::string user;
  bool view;
};

}  // namespace

namespace panel {
Panel passwd() {
  return Make<passwdImpl>();
}

}  // namespace panel

}  // namespace ui