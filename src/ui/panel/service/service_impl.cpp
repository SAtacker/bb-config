#include "ftxui/component/component.hpp"
#include "ui/panel/panel.hpp"
#include "utils.hpp"

#include <gio/gio.h>
#include <string>
#include <sstream>
#include <time.h>

using namespace ftxui;

struct service {
    std::string name;
    std::string path;
    bool status;
    bool backUp;
};

namespace ui {

namespace {

class ServiceImpl : public PanelBase {
    public:
        ServiceImpl(ScreenInteractive* screen) : screen_(screen) {
            get_services();
            build_ui();

            Add(Container::Tab({
                Container::Vertical({
                    container_,
                    button_,
                }),
                Container::Vertical({

                }),
            }, &tab_selected_));
        }

        ~ServiceImpl() {
            if (thread_.joinable())
                thread_.join();
        }

        std::string Title() override { return "Services"; }

    private:
        void get_services() {
            list_services_.clear();
            GError* error = NULL;
            GVariant *ret;
            char *path, *status;
            GDBusProxy *proxy;
            GVariantIter *iter;

            /* Create a D-Bus proxy; NM_DBUS_* defined in nm-dbus-interface.h */
            proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
                                                G_DBUS_PROXY_FLAGS_NONE,
                                                NULL,
                                                "org.freedesktop.systemd1",
                                                "/org/freedesktop/systemd1",
                                                "org.freedesktop.systemd1.Manager",
                                                NULL,
                                                NULL);

            g_assert(proxy);

            ret = g_dbus_proxy_call_sync(
                proxy,
                "ListUnitFiles",
                NULL,
                G_DBUS_CALL_FLAGS_NONE,
                -1,
                NULL,
                &error
            );

            if (!ret) {
                g_dbus_error_strip_remote_error(error);
                g_warning("Failed to get list connections: %s\n", error->message);
                g_error_free(error);
                return;
            }

            g_variant_get(ret, "(a(ss))", &iter);
            while (g_variant_iter_loop(iter, "(ss)", &path, &status)) {
                std::string s_path(path);
                std::string s_status(status);
                struct service temp;
                std::stringstream ss(s_path);

                temp.path = s_path;
                while(ss.good()) {
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

            if (iter)
                g_variant_iter_free (iter);
            if (ret)
                g_variant_unref(ret);
            if (proxy)
                g_object_unref(proxy);
        }

        void build_ui() {
            container_->DetachAllChildren();
            for (auto& it : list_services_) {
                container_->Add(Checkbox(it.name, &it.status));
            }
        }
        
        void enable_service(const std::string &name) {
            GError* error = NULL;
            GVariant *ret, *value;
            GDBusProxy *proxy;
            GVariantBuilder *builder;

            /* Create a D-Bus proxy; NM_DBUS_* defined in nm-dbus-interface.h */
            proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
                                                G_DBUS_PROXY_FLAGS_NONE,
                                                NULL,
                                                "org.freedesktop.systemd1",
                                                "/org/freedesktop/systemd1",
                                                "org.freedesktop.systemd1.Manager",
                                                NULL,
                                                NULL);

            g_assert(proxy);

            builder = g_variant_builder_new (G_VARIANT_TYPE ("as"));
            g_variant_builder_add (builder, "s", name.c_str());
            value = g_variant_new ("(asbb)", builder, FALSE, FALSE);

            ret = g_dbus_proxy_call_sync(
                proxy,
                "EnableUnitFiles",
                value,
                G_DBUS_CALL_FLAGS_NONE,
                -1,
                NULL,
                &error
            );

            if (!ret) {
                g_dbus_error_strip_remote_error(error);
                g_warning("Failed to get disabled_service: %s\n", error->message);
                g_error_free(error);
                return;
            } else {
                g_variant_unref(ret);
            }

            if (proxy)
                g_object_unref(proxy);
        }
        
        void disable_service(const std::string &name) {
            GError* error = NULL;
            GVariant *ret, *value;
            GDBusProxy *proxy;
            GVariantBuilder *builder;

            /* Create a D-Bus proxy; NM_DBUS_* defined in nm-dbus-interface.h */
            proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
                                                G_DBUS_PROXY_FLAGS_NONE,
                                                NULL,
                                                "org.freedesktop.systemd1",
                                                "/org/freedesktop/systemd1",
                                                "org.freedesktop.systemd1.Manager",
                                                NULL,
                                                NULL);

            g_assert(proxy);

            builder = g_variant_builder_new (G_VARIANT_TYPE ("as"));
            g_variant_builder_add (builder, "s", name.c_str());
            value = g_variant_new ("(asb)", builder, FALSE);

            ret = g_dbus_proxy_call_sync(
                proxy,
                "DisableUnitFiles",
                value,
                G_DBUS_CALL_FLAGS_NONE,
                -1,
                NULL,
                &error
            );

            if (!ret) {
                g_dbus_error_strip_remote_error(error);
                g_warning("Failed to get disabled_service: %s\n", error->message);
                g_error_free(error);
                return;
            } else {
                g_variant_unref(ret);
            }

            if (proxy)
                g_object_unref(proxy);
        }
        
        void apply_setting() {
            if (thread_.joinable())
                thread_.join();

            tab_selected_ = 1;
            thread_ = std::thread([=] {
                for (const auto &it : list_services_) {
                    if (it.status != it.backUp) {
                        if (it.status)
                            enable_service(it.name);
                        else
                            disable_service(it.name);
                    }
                }
                sleep(2);
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
                container_->Render() | vscroll_indicator | frame | size(HEIGHT, LESS_THAN, 20),
                separator(),
                button_->Render(),
            });

            Element loading = vbox({
                text("Waiting for processing ...") | bold
            });

            return (tab_selected_ == 0) ? page : loading;
        }

        int tab_selected_ = 0;
        std::vector<struct service> list_services_;
        ScreenInteractive* screen_;
        std::thread thread_;
        Component container_ = Container::Vertical({});
        Component waiting_ = Container::Vertical({});
        Component button_ = Button("Apply", [this] { apply_setting();});
        Receiver<decltype(list_services_)> list_receiver_ = MakeReceiver<decltype(list_services_)>();
};

}  // namespace

namespace panel {
Panel service(ScreenInteractive* screen) {
    return Make<ServiceImpl>(screen);
}

}  // namespace panel

}  // namespace ui