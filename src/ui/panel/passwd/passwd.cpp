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
  passwdImpl() : user_(getlogin()) {
    input_options_.password = &hide_password_;
    input_password_old_ =
        Input(&password_old_, "Current Password", &input_options_);
    input_password_new_ =
        Input(&password_new_, "New Password", &input_options_);
    hide_password_checkbox_ = Checkbox("Hide password", &hide_password_);
    apply_button_ = Button("Apply", [&] {
      auto cmd = "passwd " + user_;
      FILE* fp = popen(cmd.c_str(), "w");
      fprintf(fp, "%s\n", password_old_.data());
      fprintf(fp, "%s\n", password_new_.data());
      fprintf(fp, "%s\n", password_new_.data());
      pclose(fp);
    });

    auto layout = Container::Vertical({
        input_password_old_,
        input_password_new_,
        hide_password_checkbox_,
        apply_button_,
    });

    layout = Renderer(layout, [&] {
      return vbox({
          hbox(text("User:"), text(user_)),
          hbox(text("Old password:"), input_password_old_->Render()),
          hbox(text("New password:"), input_password_new_->Render()),
          hide_password_checkbox_->Render(),
          apply_button_->Render(),
      });
    });

    Add(layout);
  }
  ~passwdImpl() = default;
  std::string Title() override { return "Password"; }

 private:
  std::string password_old_;
  std::string password_new_;
  std::string user_;

  Component input_password_old_;
  Component input_password_new_;
  Component hide_password_checkbox_;
  Component apply_button_;
  InputOption input_options_;
  bool hide_password_ = false;
};

}  // namespace

namespace panel {
Panel passwd() {
  return Make<passwdImpl>();
}

}  // namespace panel

}  // namespace ui
