#include "ui.hpp"

Element ui::ColorTile(int red, int green, int blue) {
  return text(L"") | size(WIDTH, GREATER_THAN, 14) |
         size(HEIGHT, GREATER_THAN, 7) | bgcolor(Color::RGB(red, green, blue));
}

Element ui::ColorString(int red, int green, int blue) {
  return text(L"RGB = (" +                    //
              std::to_wstring(red) + L"," +   //
              std::to_wstring(green) + L"," + //
              std::to_wstring(blue) + L")"    //
  );
}

ui::MyComponent::MyComponent(int *red, int *green, int *blue,
                         std::function<void(void)> quit)
    : red_(red), green_(green), blue_(blue), quit_(quit) {
  Add(&container_);
  container_.Add(slider_red_.get());
  container_.Add(slider_green_.get());
  container_.Add(slider_blue_.get());
}

Element ui::MyComponent::Render() {
  return hbox({
             ColorTile(*red_, *green_, *blue_),
             separator(),
             vbox({
                 slider_red_->Render(),
                 separator(),
                 slider_green_->Render(),
                 separator(),
                 slider_blue_->Render(),
                 separator(),
                 ColorString(*red_, *green_, *blue_),
             }) | xflex,
         }) |
         border | size(WIDTH, LESS_THAN, 80);
}

bool ui::MyComponent::OnEvent(Event event) {
  if (event == Event::Return || event == Event::Escape)
    quit_();
  return Component::OnEvent(event);
}
