#include <thread>
#include "ftxui/component/component.hpp"
#include "ftxui/component/event.hpp"
#include "ftxui/component/receiver.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/dom/elements.hpp"
#include "ui/panel/panel.hpp"

using namespace ftxui;
using namespace std::chrono_literals;

namespace ui {

void WorkerMain0(bool* should_continue,
                 std::function<void()> refresh_ui,
                 Sender<std::wstring> out) {
  out->Send(L"Worker 0 starting...");
  refresh_ui();

  for (int i = 0; i < 100 && *should_continue; ++i) {
    std::wstring index = std::to_wstring(i);
    std::this_thread::sleep_for(0.05s);
    out->Send(L"Worker 0 working... (" + index + L"/100)");
    refresh_ui();
  }

  out->Send(L"Worker 0 exiting...");
  refresh_ui();
}

void WorkerMain1(bool* should_continue,
                 std::function<void()> refresh_ui,
                 Sender<std::wstring> out) {
  out->Send(L"Worker 1 starting...");
  refresh_ui();

  for (int i = 0; i < 10 && *should_continue; ++i) {
    std::wstring index = std::to_wstring(i);
    std::this_thread::sleep_for(0.5s);
    out->Send(L"Worker 1 working... (" + index + L"/10)");
    refresh_ui();
  }

  out->Send(L"Worker 1 exiting...");
  refresh_ui();
}

// A basic implementation of PanelBase with no internal logic.
class BackgroundWorkerImpl : public PanelBase {
 public:
  BackgroundWorkerImpl(ScreenInteractive* screen) : screen_(screen) {
    ButtonOption options;
    options.border = false;
    button_[0] = Button(
        &button_label_[0], [this] { ToggleWorker(0); }, &options);
    button_[1] = Button(
        &button_label_[1], [this] { ToggleWorker(1); }, &options);
    Add(Container::Vertical({
        button_[0],
        button_[1],
    }));
  };
  ~BackgroundWorkerImpl() {
    // Stop every workers.
    for (int i : {0, 1}) {
      if (worker_thread_[i].joinable()) {
        worker_continue_[i] = false;
        worker_thread_[i].join();
      }
    }
  }
  std::wstring Title() override { return L"background worker demo"; }

  Element Render() override {
    button_label_[0] =
        std::wstring(L"Worker 0 ") +
        (worker_thread_[0].joinable() ? L"(Running)" : L"(Stopped)");
    button_label_[1] =
        std::wstring(L"Worker 1 ") +
        (worker_thread_[1].joinable() ? L"(Running)" : L"(Stopped)");

    // Check for new entries received.
    while (receiver_->HasPending()) {
      std::wstring str;
      receiver_->Receive(&str);
      received_.push_back(str);
    }

    Elements received_list;
    for (const auto& item : received_)
      received_list.push_back(text(item));
    received_list.push_back(text(L"...") | ftxui::select);

    return vbox({
               button_[0]->Render(),
               button_[1]->Render(),
               separator(),
               vbox(std::move(received_list)) | yframe | yflex,
           }) |
           flex;
  }

 private:
  void ToggleWorker(int index) {
    if (worker_thread_[index].joinable()) {
      worker_continue_[index] = false;
      worker_thread_[index].join();
    } else {
      worker_continue_[index] = true;
      std::function<void()> refresh_ui = [&] {
        screen_->PostEvent(Event::Custom);
      };
      auto WorkerMain = index ? WorkerMain1 : WorkerMain0;
      worker_thread_[index] = std::thread(WorkerMain, &worker_continue_[index],
                                          refresh_ui, receiver_->MakeSender());
    }
  }

  ScreenInteractive* screen_;

  Component button_[2];
  std::thread worker_thread_[2];
  std::wstring button_label_[2];
  bool worker_continue_[2] = {false, false};
  Receiver<std::wstring> receiver_ = MakeReceiver<std::wstring>();
  std::vector<std::wstring> received_;
};

namespace panel {
Panel BackgroundWorker(ScreenInteractive* screen) {
  return Make<BackgroundWorkerImpl>(screen);
}

}  // namespace panel
}  // namespace ui
