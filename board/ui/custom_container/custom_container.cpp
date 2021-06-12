#include "custom_container.hpp"

namespace custom_container_component {

Component panel(Components components,
                std::vector<std::wstring> titles,
                int sel,
                int* external) {
  return Make<panel_base>(components, titles, sel, external);
}

// static
panel_base* panel_base::From(Component component) {
  return static_cast<panel_base*>(component.get());
}

panel_base::panel_base(Components components,
                       std::vector<std::wstring> titles,
                       int sel = 0,
                       int* external = nullptr)
    : titles(titles), components(components), sel(sel), external(external) {
  if (sel == 0 && external == nullptr)
    Add(Container::Vertical(components));
  else if (sel == 1 && external == nullptr)
    Add(Container::Horizontal(components));
  else if (external != nullptr && sel == 0) {
    auto cont = Container::Tab(components, external);
    cont->Add(Container::Vertical(components));
    Add(std::move(cont));
  } else if (external != nullptr && sel == 1) {
    auto cont = Container::Tab(components, external);
    cont->Add(Container::Horizontal(components));
    Add(std::move(cont));
  }
}

Element panel_base::Render() {
  Elements elements;
  for (size_t i = 0; i < components.size(); i++) {
    if (components.at(i)->Active()) {
      if (external != nullptr && components.at(i)->Focused())
        *external = i;
      elements.push_back(window(
          text(titles.at(i)),
          components.at(i)->Render() | bold | color(Color::Chartreuse1)));
    } else {
      elements.push_back(
          window(text(titles.at(i)), components.at(i)->Render() | nothing));
    }
  }
  if (sel == 0)
    return vbox(std::move(elements));

  return hbox(std::move(elements));
}

}  // namespace custom_container_component
