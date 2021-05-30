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
    : titles(titles), components(components), sel(sel), external(external) {}

Element panel_base::Render() {
  Elements elements;
  for (size_t i = 0; i < components.size(); i++) {
    if (i == selected_) {
      if (external != nullptr)
        *external = selected_;
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

bool panel_base::VerticalEvent(Event event) {
  int old_selected = selected_;
  if (event == Event::ArrowUp || event == Event::Character('k'))
    selected_--;
  if (event == Event::ArrowDown || event == Event::Character('j'))
    selected_++;
  if (event == Event::Tab && components.size())
    selected_ = (selected_ + 1) % components.size();
  if (event == Event::TabReverse && components.size())
    selected_ = (selected_ + components.size() - 1) % components.size();

  selected_ = std::max(0, std::min(int(components.size()) - 1, selected_));
  return old_selected != selected_;
}

bool panel_base::HorizontalEvent(Event event) {
  int old_selected = selected_;
  if (event == Event::ArrowLeft || event == Event::Character('h'))
    selected_--;
  if (event == Event::ArrowRight || event == Event::Character('l'))
    selected_++;
  if (event == Event::Tab && components.size())
    selected_ = (selected_ + 1) % components.size();
  if (event == Event::TabReverse && components.size())
    selected_ = (selected_ + components.size() - 1) % components.size();

  selected_ = std::max(0, std::min(int(components.size()) - 1, selected_));
  return old_selected != selected_;
}

bool panel_base::OnEvent(Event event) {
  if (event.is_mouse())
    return OnMouseEvent(event);

  if (!Focused())
    return false;

  if (ActiveChild() && ActiveChild()->OnEvent(event))
    return true;

  if (sel == 0)
    return (this->VerticalEvent)(event);

  return (this->HorizontalEvent)(event);
}

Component panel_base::ActiveChild() {
  if (components.size() == 0)
    return nullptr;

  int selected = selector_ ? *selector_ : selected_;
  return components[selected % components.size()];
}

void panel_base::SetActiveChild(ComponentBase* child) {
  for (size_t i = 0; i < components.size(); ++i) {
    if (components[i].get() == child) {
      (selector_ ? *selector_ : selected_) = i;
      return;
    }
  }
}

bool panel_base::OnMouseEvent(Event event) {
  if (selector_)
    return ActiveChild()->OnEvent(event);

  for (Component& child : components) {
    if (child->OnEvent(event))
      return true;
  }
  return false;
}

}  // namespace custom_container_component
