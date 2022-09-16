#include "ftxui/component/component.hpp"
#include "ui/panel/panel.hpp"
#include "utils.hpp"

/*
 * GIO is a library, designed to present programmers
 * with a modern and usable interface to a virtual
 * file system.
 */
#include <gio/gio.h>
#include <sstream>
#include <string>

using namespace ftxui;

/*  store UnitListFiles services */
struct service {
  std::string name; /* service name */
  std::string path; /* service files path */
  bool status;      /* status enable = true */
  bool backUp;      /* keep track changes */
};

namespace ui {

namespace {

class ServiceImpl : public PanelBase {
 public:
  ServiceImpl(ScreenInteractive* screen) : screen_(screen) {
    get_services();
    build_ui();

    Add(Container::Tab(
        {
            Container::Vertical({
                container_,
                button_,
            }),
            Container::Vertical({

            }),
        },
        &tab_selected_));
  }

  ~ServiceImpl() {
    /* When exit the program disable all
     * active threads
     */
    if (thread_.joinable())
      thread_.join();
  }

  std::string Title() override { return "Services"; }

 private:
  /*
   * This function read all the avaliable services
   * in the systemd. Services will store inside list_services_
   */
  void get_services() {
    list_services_.clear();

    /* Initialize all the variable */
    GError* error = NULL;
    GVariant* ret;
    char *path, *status;
    GDBusProxy* proxy;
    GVariantIter* iter;

    /* Create a D-Bus proxy for systemd */
    proxy = g_dbus_proxy_new_for_bus_sync(
        G_BUS_TYPE_SYSTEM, G_DBUS_PROXY_FLAGS_NONE, NULL,
        "org.freedesktop.systemd1", "/org/freedesktop/systemd1",
        "org.freedesktop.systemd1.Manager", NULL, NULL);

    g_assert(proxy);

    /* Call ListUnitFiles method */
    ret = g_dbus_proxy_call_sync(proxy, "ListUnitFiles", NULL,
                                 G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);

    /* Error Handling */
    if (!ret) {
      g_dbus_error_strip_remote_error(error);
      g_warning("Failed to get list connections: %s\n", error->message);
      g_error_free(error);
      return;
    }

    /* store the return info into vector */
    g_variant_get(ret, "(a(ss))", &iter);
    while (g_variant_iter_loop(iter, "(ss)", &path, &status)) {
      std::string s_path(path);
      std::string s_status(status);
      struct service temp;
      std::stringstream ss(s_path);

      temp.path = s_path;
      while (ss.good()) {
        getline(ss, s_path, '/');
      }
      temp.name = s_path;

      if (!s_status.compare("enabled"))
        temp.status = true;
      else if (!s_status.compare("disabled"))
        temp.status = false;
      else
        continue;

      temp.backUp = temp.status;
      list_services_.push_back(temp);
    }

    /* free the memory after used */
    if (iter)
      g_variant_iter_free(iter);
    if (ret)
      g_variant_unref(ret);
    if (proxy)
      g_object_unref(proxy);
  }

  /* Create a list of checkbox for the services */
  void build_ui() {
    container_->DetachAllChildren();
    for (auto& it : list_services_) {
      container_->Add(Checkbox(it.name, &it.status));
    }
  }

  /* Enable the service to start when booting */
  void enable_service(const std::string& name) {
    GError* error = NULL;
    GVariant *ret, *value;
    GDBusProxy* proxy;
    GVariantBuilder* builder;

    /* Create a D-Bus proxy for systemd */
    proxy = g_dbus_proxy_new_for_bus_sync(
        G_BUS_TYPE_SYSTEM, G_DBUS_PROXY_FLAGS_NONE, NULL,
        "org.freedesktop.systemd1", "/org/freedesktop/systemd1",
        "org.freedesktop.systemd1.Manager", NULL, NULL);

    g_assert(proxy);

    /* Create a GVarient which consist name of services */
    builder = g_variant_builder_new(G_VARIANT_TYPE("as"));
    g_variant_builder_add(builder, "s", name.c_str());
    value = g_variant_new("(asbb)", builder, FALSE, FALSE);

    /* Call EnableUnitFiles function to enable the service */
    ret = g_dbus_proxy_call_sync(proxy, "EnableUnitFiles", value,
                                 G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);

    /* Error Handling */
    if (!ret) {
      g_dbus_error_strip_remote_error(error);
      g_warning("Failed to get disabled_service: %s\n", error->message);
      g_error_free(error);
      return;
    } else {
      g_variant_unref(ret);
    }

    /* free the memory after used */
    if (proxy)
      g_object_unref(proxy);
  }

  /* Disable the service to start when booting */
  void disable_service(const std::string& name) {
    GError* error = NULL;
    GVariant *ret, *value;
    GDBusProxy* proxy;
    GVariantBuilder* builder;

    /* Create a D-Bus proxy for systemd */
    proxy = g_dbus_proxy_new_for_bus_sync(
        G_BUS_TYPE_SYSTEM, G_DBUS_PROXY_FLAGS_NONE, NULL,
        "org.freedesktop.systemd1", "/org/freedesktop/systemd1",
        "org.freedesktop.systemd1.Manager", NULL, NULL);

    g_assert(proxy);

    /* Call EnableUnitFiles function to enable the service */
    builder = g_variant_builder_new(G_VARIANT_TYPE("as"));
    g_variant_builder_add(builder, "s", name.c_str());
    value = g_variant_new("(asb)", builder, FALSE);

    /* Call EnableUnitFiles function to enable the service */
    ret = g_dbus_proxy_call_sync(proxy, "DisableUnitFiles", value,
                                 G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);

    /* Error Handling */
    if (!ret) {
      g_dbus_error_strip_remote_error(error);
      g_warning("Failed to get disabled_service: %s\n", error->message);
      g_error_free(error);
      return;
    } else {
      g_variant_unref(ret);
    }

    /* free the memory after used */
    if (proxy)
      g_object_unref(proxy);
  }

  void apply_setting() {
    if (thread_.joinable())
      thread_.join();

    /* prompt loading page when applying the changes */
    tab_selected_ = 1;
    thread_ = std::thread([=] {
      for (const auto& it : list_services_) {
        if (it.status != it.backUp) {
          if (it.status)
            enable_service(it.name);
          else
            disable_service(it.name);
        }
      }
      get_services();
      build_ui();
      tab_selected_ = 0;
      screen_->PostEvent(Event::Custom);
    });
  }

  Element Render() override {
    Element page = vbox({
        text("[!!] Requires sudo or root permision.") | color(Color::Red),
        separator(),
        container_->Render() | vscroll_indicator | frame |
            size(HEIGHT, LESS_THAN, 20),
        separator(),
        button_->Render(),
    });

    Element loading = vbox({text("Waiting for processing ...") | bold});

    return (tab_selected_ == 0) ? page : loading;
  }

  int tab_selected_ = 0;
  std::vector<struct service> list_services_;
  ScreenInteractive* screen_;
  std::thread thread_;
  Component container_ = Container::Vertical({});
  Component waiting_ = Container::Vertical({});
  Component button_ = Button("Apply", [this] { apply_setting(); });
};

}  // namespace

namespace panel {
Panel service(ScreenInteractive* screen) {
  return Make<ServiceImpl>(screen);
}

}  // namespace panel

}  // namespace ui