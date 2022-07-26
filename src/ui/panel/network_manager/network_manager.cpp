#include "network_manager.hpp"

NetworkManager::NetworkManager() {
    wifi_init();
}

void NetworkManager::wifi_init() {
    update_wifi_path();
    update_network_enable();
    update_wireless_accesspoints();
    update_wifi_activeConnection();
    update_saved_accesspoints();
    update_new_accesspoint();
}

void NetworkManager::update_wifi_path() {
    GDBusProxy *props_proxy;
    GError *  error = NULL;
    GVariant *ret = NULL, *value = NULL;
    char ** paths = NULL;

    props_proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
                                                G_DBUS_PROXY_FLAGS_NONE,
                                                NULL,
                                                NM_DBUS_SERVICE,
                                                NM_DBUS_PATH,
                                                "org.freedesktop.DBus.Properties",
                                                NULL,
                                                NULL);
    g_assert(props_proxy);

    ret = g_dbus_proxy_call_sync(props_proxy,
                                "Get",
                                g_variant_new("(ss)", NM_DBUS_INTERFACE, "AllDevices"),
                                G_DBUS_CALL_FLAGS_NONE,
                                -1,
                                NULL,
                                &error);

    if (!ret) {
        g_dbus_error_strip_remote_error(error);
        g_warning("Failed to get AllDevices property: %s\n", error->message);
        g_error_free(error);
        return;
    }

    g_variant_get(ret, "(v)", &value);

    if (!g_variant_is_of_type(value, G_VARIANT_TYPE("ao"))) {
        g_warning("Unexpected type returned getting AllDevices: %s",
                g_variant_get_type_string(value));
        if (value)
            g_variant_unref(value);
        if (ret)
            g_variant_unref(ret);
    }

    paths = g_variant_dup_objv(value, NULL);

    if (!paths) {
        g_warning("Could not retrieve device path property");
        if (value)
            g_variant_unref(value);
        if (ret)
            g_variant_unref(ret);
    }

    for (int i = 0; paths[i]; i++) {
        if (get_device_types(paths[i]) == 2) {
            wifi_objects.wifi_avaliable = true;
            wifi_objects.wifi_device_path = paths[i];
        }
    }

    g_strfreev(paths);
    g_variant_unref(ret);
}

int NetworkManager::get_device_types(char* dev_path) {
    GDBusProxy *props_proxy;
    GVariant *  ret = NULL, * path_value = NULL;
    GError *    error = NULL;
    int dev_type_int;

    props_proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
                                G_DBUS_PROXY_FLAGS_NONE,
                                NULL,
                                NM_DBUS_SERVICE,
                                dev_path,
                                "org.freedesktop.DBus.Properties",
                                NULL,
                                NULL);

    g_assert(props_proxy);
    
    ret = g_dbus_proxy_call_sync(props_proxy,
                                "Get",
                                g_variant_new("(ss)", NM_DBUS_INTERFACE_DEVICE, "DeviceType"),
                                G_DBUS_CALL_FLAGS_NONE,
                                -1,
                                NULL,
                                &error);

    g_variant_get(ret, "(v)", &path_value);
    dev_type_int = g_variant_get_uint32(path_value);

    g_variant_unref(ret);

    return dev_type_int;
}

void NetworkManager::read_access_point_detail(const char *path, char *ssid_name, uint8_t *strength)
{
    GDBusProxy * proxy;
    GError *     error = NULL;
    GVariant *   ret;
    GVariant * path_value;
    GVariantIter iter;
    char paths;

    proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
                                        G_DBUS_PROXY_FLAGS_NONE,
                                        NULL,
                                        NM_DBUS_SERVICE,
                                        path,
                                        "org.freedesktop.DBus.Properties",
                                        NULL,
                                        NULL);
    g_assert(proxy);

    ret = g_dbus_proxy_call_sync(proxy,
                                "Get",
                                g_variant_new("(ss)", NM_DBUS_INTERFACE_ACCESS_POINT, "Ssid"),
                                G_DBUS_CALL_FLAGS_NONE,
                                -1,
                                NULL,
                                &error);
    if (!ret) {
        g_dbus_error_strip_remote_error(error);
        g_warning("Failed to get Ssid: %s\n", error->message);
        g_error_free(error);
        return;
    }

    g_variant_get(ret, "(v)", &path_value);

    g_variant_iter_init(&iter, path_value);

    int len = 0;
    while (g_variant_iter_next(&iter, "y", &paths)) {
        ssid_name[len++] = paths;
    }
    ssid_name[len] = '\0';

    ret = g_dbus_proxy_call_sync(proxy,
                                "Get",
                                g_variant_new("(ss)", NM_DBUS_INTERFACE_ACCESS_POINT, "Strength"),
                                G_DBUS_CALL_FLAGS_NONE,
                                -1,
                                NULL,
                                &error);
    if (!ret) {
        g_dbus_error_strip_remote_error(error);
        g_warning("Failed to get Strength: %s\n", error->message);
        g_error_free(error);
        return;
    }

    g_variant_get(ret, "(v)", &path_value);
    *strength = g_variant_get_byte(path_value);

    if (path_value)
        g_variant_unref(path_value);
    if (ret)
        g_variant_unref(ret);

}

void NetworkManager::read_save_accessPoint_detail(const char *path, const int ite) {
    GDBusProxy * proxy;
    GError *     error = NULL;
    GVariant *   ret, *connection = NULL, *s_con = NULL;
    const char *id, *type;
    gboolean     found;
    GVariantIter iter;
    const char * setting_name;
    GVariant *   setting;

    /* This function asks NetworkManager for the details of the connection */

    /* Create the D-Bus proxy so we can ask it for the connection configuration details. */
    proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
                                          G_DBUS_PROXY_FLAGS_NONE,
                                          NULL,
                                          NM_DBUS_SERVICE,
                                          path,
                                          NM_DBUS_INTERFACE_SETTINGS_CONNECTION,
                                          NULL,
                                          NULL);
    g_assert(proxy);

    /* Request the all the configuration of the Connection */
    ret = g_dbus_proxy_call_sync(proxy,
                                 "GetSettings",
                                 NULL,
                                 G_DBUS_CALL_FLAGS_NONE,
                                 -1,
                                 NULL,
                                 &error);
    if (!ret) {
        g_dbus_error_strip_remote_error(error);
        g_warning("Failed to get connection settings: %s\n", error->message);
        g_error_free(error);

        return;
    }

    g_variant_get(ret, "(@a{sa{sv}})", &connection);

    s_con = g_variant_lookup_value(connection, NM_SETTING_CONNECTION_SETTING_NAME, NULL);
    g_assert(s_con != NULL);
    found = g_variant_lookup(s_con, NM_SETTING_CONNECTION_ID, "&s", &id);
    g_assert(found);
    found = g_variant_lookup(s_con, NM_SETTING_CONNECTION_TYPE, "&s", &type);
    g_assert(found);

    wifi_objects.saved_access_points[ite].ssid = id;

    /* Then the type-specific setting */
    setting = g_variant_lookup_value(connection, type, NULL);
    if (setting) {
        g_variant_unref(setting);
    }

    g_variant_iter_init(&iter, connection);
    while (g_variant_iter_next(&iter, "{&s@a{sv}}", &setting_name, &setting)) {
        g_variant_unref(setting);
    }

    if (s_con)
        g_variant_unref(s_con);
    if (connection)
        g_variant_unref(connection);
    if (ret)
        g_variant_unref(ret);
    g_object_unref(proxy);
}

void NetworkManager::update_wifi_activeConnection() {
    GDBusProxy *props_proxy;
    GVariant *  ret = NULL, *value = NULL;
    GError *    error = NULL;
    char * path = NULL;
    char ** paths = NULL;
    char ssid[225];
    int  n;
    uint8_t strength;

    n = wifi_objects.wifi_device_path.length();
    char *dev_path = new char[n + 1];
    strcpy(dev_path, wifi_objects.wifi_device_path.c_str());

    props_proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
                                            G_DBUS_PROXY_FLAGS_NONE,
                                            NULL,
                                            NM_DBUS_SERVICE,
                                            dev_path,
                                            "org.freedesktop.DBus.Properties",
                                            NULL,
                                            NULL);
    
    g_assert(props_proxy);

    ret = g_dbus_proxy_call_sync(props_proxy,
                                "Get",
                                g_variant_new("(ss)", NM_DBUS_INTERFACE_DEVICE_WIRELESS, "ActiveAccessPoint"),
                                G_DBUS_CALL_FLAGS_NONE,
                                -1,
                                NULL,
                                &error);
    
    g_variant_get(ret, "(v)", &value);

    if (!g_variant_is_of_type(value, G_VARIANT_TYPE("o"))) {
        g_warning("Unexpected type returned getting ActiveAccessPoint: %s",
                g_variant_get_type_string(value));
        if (value)
            g_variant_unref(value);
        if (ret)
            g_variant_unref(ret);
    }

    path = g_variant_dup_string(value, NULL);

    if (!path) {
        g_warning("Could not retrieve device path property");
        if (value)
            g_variant_unref(value);
        if (ret)
            g_variant_unref(ret);
    }
    
    if (strcmp(path, "/") != 0)
        read_access_point_detail(path, ssid, &strength);

    props_proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
                                        G_DBUS_PROXY_FLAGS_NONE,
                                        NULL,
                                        NM_DBUS_SERVICE,
                                        NM_DBUS_PATH,
                                        "org.freedesktop.DBus.Properties",
                                        NULL,
                                        &error);
    
    g_assert(props_proxy);

    ret = g_dbus_proxy_call_sync(props_proxy,
                                "Get",
                                g_variant_new("(ss)", NM_DBUS_INTERFACE, "ActiveConnections"),
                                G_DBUS_CALL_FLAGS_NONE,
                                -1,
                                NULL,
                                &error);

    if (ret) {
        g_variant_get(ret, "(v)", &value);
        paths = g_variant_dup_objv(value, NULL);
        g_variant_unref(ret);
    } else {
        g_dbus_error_strip_remote_error(error);
        g_print("Error adding connection: %s\n", error->message);
        g_clear_error(&error);
    }

    for (int i = 0; paths[i]; i++) {
        bool check = check_wifi_type_activeConnections(paths[i]);
        if (check) {
            wifi_objects.active_access_point.aceesspoint_dir = paths[i];
            break;
        }
    }

    g_strfreev(paths);
    g_object_unref(props_proxy);

    wifi_objects.active_access_point.ssid = ssid;
    wifi_objects.active_access_point.strength = strength;

    g_free(path);
}

bool NetworkManager::check_wifi_type_activeConnections(const char * active_path) {
    GDBusProxy *props_proxy;
    GVariant *  ret = NULL, *value = NULL;
    GError *    error = NULL;
    char ** paths = NULL;

    props_proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
                                        G_DBUS_PROXY_FLAGS_NONE,
                                        NULL,
                                        NM_DBUS_SERVICE,
                                        active_path,
                                        "org.freedesktop.DBus.Properties",
                                        NULL,
                                        &error);
    
    g_assert(props_proxy);

    ret = g_dbus_proxy_call_sync(props_proxy,
                                "Get",
                                g_variant_new("(ss)", NM_DBUS_INTERFACE_ACTIVE_CONNECTION, "Devices"),
                                G_DBUS_CALL_FLAGS_NONE,
                                -1,
                                NULL,
                                &error);

    if (ret) {
        g_variant_get(ret, "(v)", &value);
        paths = g_variant_dup_objv(value, NULL);
        g_variant_unref(ret);
    } else {
        g_dbus_error_strip_remote_error(error);
        g_print("Error adding connection: %s\n", error->message);
        g_clear_error(&error);
    }

    for (int i = 0; paths[i]; i++) 
    {
        if (wifi_objects.wifi_device_path.compare(paths[i]) == 0)
            return true;
        else    
            return false;
    }
    return false;
}

void NetworkManager::update_wireless_accesspoints() {
    GDBusProxy *props_proxy;
    GVariant *  ret = NULL;
    GError *    error = NULL;
    uint8_t strength;

    int       i;
    char **   paths;

    int n = wifi_objects.wifi_device_path.length();
    char *dev_path = new char[n + 1];
    strcpy(dev_path, wifi_objects.wifi_device_path.c_str());

    props_proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
                                            G_DBUS_PROXY_FLAGS_NONE,
                                            NULL,
                                            NM_DBUS_SERVICE,
                                            dev_path,
                                            NM_DBUS_INTERFACE_DEVICE_WIRELESS,
                                            NULL,
                                            NULL);
    g_assert(props_proxy);

    ret = g_dbus_proxy_call_sync(props_proxy,
                                "GetAllAccessPoints",
                                NULL,
                                G_DBUS_CALL_FLAGS_NONE,
                                -1,
                                NULL,
                                &error);
    
    if (!ret) {
        g_dbus_error_strip_remote_error(error);
        g_print("ListConnections failed: %s\n", error->message);
        g_error_free(error);
        return;
    }

    g_variant_get(ret, "(^ao)", &paths);
    g_variant_unref(ret);

    for (i = 0; paths[i]; i++) {
        Accesspoint_Objects access_point;
        char ssid[225];
        read_access_point_detail(paths[i], ssid, &strength);
        access_point.aceesspoint_dir = paths[i];
        access_point.ssid = ssid;
        access_point.strength = strength;
        wifi_objects.access_points.push_back(access_point);
    }
}

void NetworkManager::saved_wireless(const int num_ap, const std::string password) {
    GVariantBuilder connection_builder;
    GVariantBuilder setting_builder;
    char *          uuid;
    const char *    new_con_path;
    GVariant *      ret, *ret2, *value;
    GError *        error = NULL;
    GDBusProxy *proxy, *proxy2;

    int n = wifi_objects.access_points[num_ap].aceesspoint_dir.length();
    char *dev_path = new char[n + 1];
    strcpy(dev_path, wifi_objects.access_points[num_ap].aceesspoint_dir.c_str());

    n = wifi_objects.access_points[num_ap].ssid.length();
    char *ssid_char = new char[n + 1];
    strcpy(ssid_char, wifi_objects.access_points[num_ap].ssid.c_str());

    n = password.length();
    char *password_char = new char[n + 1];
    strcpy(password_char, password.c_str());

    proxy2 = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
                                        G_DBUS_PROXY_FLAGS_NONE,
                                        NULL,
                                        NM_DBUS_SERVICE,
                                        dev_path,
                                        "org.freedesktop.DBus.Properties",
                                        NULL,
                                        NULL);

    g_assert(proxy2);

    /* Request the all the configuration of the Connection */
    ret2 = g_dbus_proxy_call_sync(proxy2,
                                "Get",
                                g_variant_new("(ss)", NM_DBUS_INTERFACE_ACCESS_POINT, "Ssid"),
                                G_DBUS_CALL_FLAGS_NONE,
                                -1,
                                NULL,
                                &error);
    if (!ret2) {
        g_dbus_error_strip_remote_error(error);
        g_warning("Failed to get Ssid (saved_wireless): %s\n", error->message);
        g_error_free(error);
        return;
    }

    g_variant_get(ret2, "(v)", &value);

    /* Create a D-Bus proxy; NM_DBUS_* defined in nm-dbus-interface.h */
    proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
                                        G_DBUS_PROXY_FLAGS_NONE,
                                        NULL,
                                        NM_DBUS_SERVICE,
                                        NM_DBUS_PATH_SETTINGS,
                                        NM_DBUS_INTERFACE_SETTINGS,
                                        NULL,
                                        &error);
    if (!proxy) {
        g_dbus_error_strip_remote_error(error);
        g_print("Could not create NewtworkManager D-Bus proxy: %s\n", error->message);
        g_error_free(error);
        return;
    }

    /* Initialize connection GVariantBuilder */
    g_variant_builder_init(&connection_builder, G_VARIANT_TYPE("a{sa{sv}}"));

    /* Build up the 'connection' Setting */
    g_variant_builder_init(&setting_builder, G_VARIANT_TYPE("a{sv}"));

    uuid = nm_utils_uuid_generate();
    g_variant_builder_add(&setting_builder,
                        "{sv}",
                        NM_SETTING_CONNECTION_UUID,
                        g_variant_new_string(uuid));
    g_free(uuid);

    g_variant_builder_add(&setting_builder,
                        "{sv}",
                        NM_SETTING_CONNECTION_ID,
                        g_variant_new_string(ssid_char));

    g_variant_builder_add(&setting_builder,
                        "{sv}",
                        NM_SETTING_CONNECTION_TYPE,
                        g_variant_new_string(NM_SETTING_WIRELESS_SETTING_NAME));

    g_variant_builder_add(&connection_builder,
                        "{sa{sv}}",
                        NM_SETTING_CONNECTION_SETTING_NAME,
                        &setting_builder);

    /* Add the (empty) 'wired' Setting */
    g_variant_builder_init(&setting_builder, G_VARIANT_TYPE("a{sv}"));
    g_variant_builder_add(&setting_builder,
                        "{sv}",
                        NM_SETTING_WIRELESS_SSID,
                        value);

    g_variant_builder_add(&connection_builder,
                        "{sa{sv}}",
                        NM_SETTING_WIRELESS_SETTING_NAME,
                        &setting_builder);

    /* Build up the 'Password' Setting */
    g_variant_builder_init(&setting_builder, G_VARIANT_TYPE("a{sv}"));
    g_variant_builder_add(&setting_builder,
                        "{sv}",
                        NM_SETTING_WIRELESS_SECURITY_KEY_MGMT,
                        g_variant_new_string("wpa-psk"));
    g_variant_builder_add(&setting_builder,
                        "{sv}",
                        NM_SETTING_WIRELESS_SECURITY_PSK,
                        g_variant_new_string(password_char));

    g_variant_builder_add(&connection_builder,
                        "{sa{sv}}",
                        NM_SETTING_WIRELESS_SECURITY_SETTING_NAME,
                        &setting_builder);

    /* Build up the 'ipv4' Setting */
    g_variant_builder_init(&setting_builder, G_VARIANT_TYPE("a{sv}"));
    g_variant_builder_add(&setting_builder,
                        "{sv}",
                        NM_SETTING_IP_CONFIG_METHOD,
                        g_variant_new_string(NM_SETTING_IP4_CONFIG_METHOD_AUTO));
    g_variant_builder_add(&connection_builder,
                        "{sa{sv}}",
                        NM_SETTING_IP4_CONFIG_SETTING_NAME,
                        &setting_builder);

    /* Call AddConnection with the connection dictionary as argument.
    * (g_variant_new() will consume the floating GVariant returned from
    * &connection_builder, and g_dbus_proxy_call_sync() will consume the
    * floating variant returned from g_variant_new(), so no cleanup is needed.
    */
    ret = g_dbus_proxy_call_sync(proxy,
                                "AddConnection",
                                g_variant_new("(a{sa{sv}})", &connection_builder),
                                G_DBUS_CALL_FLAGS_NONE,
                                -1,
                                NULL,
                                &error);
    if (ret) {
        g_variant_get(ret, "(&o)", &new_con_path);
        g_print("Added: %s\n", new_con_path);
        g_variant_unref(ret);
    } else {
        g_dbus_error_strip_remote_error(error);
        g_print("Error adding connection: %s\n", error->message);
        g_clear_error(&error);
    }

    if (proxy) {
        g_object_unref(proxy);
    }
    if (proxy2) {
        g_object_unref(proxy2);
    }
    if (value) {
        g_variant_unref(value);
    }
}

void NetworkManager::update_saved_accesspoints() {
    GDBusProxy *props_proxy;
    GError *     error = NULL;
    GVariant *   ret, *value;
    char ** paths;

    int n = wifi_objects.wifi_device_path.length();
    char *dev_path = new char[n + 1];
    strcpy(dev_path, wifi_objects.wifi_device_path.c_str());

    props_proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
                                        G_DBUS_PROXY_FLAGS_NONE,
                                        NULL,
                                        NM_DBUS_SERVICE,
                                        NM_DBUS_PATH_SETTINGS,
                                        "org.freedesktop.DBus.Properties",
                                        NULL,
                                        &error);
    
    g_assert(props_proxy);

    ret = g_dbus_proxy_call_sync(props_proxy,
                                "Get",
                                g_variant_new("(ss)", NM_DBUS_INTERFACE_SETTINGS, "Connections"),
                                G_DBUS_CALL_FLAGS_NONE,
                                -1,
                                NULL,
                                &error);

    if (ret) {
        g_variant_get(ret, "(v)", &value);
        paths = g_variant_dup_objv(value, NULL);
        g_variant_unref(ret);
    } else {
        g_dbus_error_strip_remote_error(error);
        g_print("Error read connection: %s\n", error->message);
        g_clear_error(&error);
    }

    for (int i = 0; paths[i]; i++) {
        Accesspoint_Objects access_point;
        access_point.aceesspoint_dir = paths[i];
        wifi_objects.saved_access_points.push_back(access_point);
        read_save_accessPoint_detail(paths[i], i);
    }
    g_strfreev(paths);
    g_object_unref(props_proxy);
}

void NetworkManager::update_new_accesspoint() {
    uint i, j;
    for (i = 0; i < wifi_objects.access_points.size(); i++) {
        for (j = 0; j < wifi_objects.saved_access_points.size(); j++) {
            if (wifi_objects.access_points[i].aceesspoint_dir.compare(wifi_objects.saved_access_points[j].aceesspoint_dir) != 0) {
                Accesspoint_Objects access_point;
                access_point.aceesspoint_dir = wifi_objects.access_points[i].aceesspoint_dir;
                access_point.ssid = wifi_objects.access_points[i].ssid;
                access_point.strength = wifi_objects.access_points[i].strength;
                wifi_objects.new_access_points.push_back(access_point);
                break;
            }
        }
    }
}

void NetworkManager::disconnect_active_WiFi() {
    GDBusProxy *props_proxy;
    GError *     error = NULL;
    GVariant *   ret;

    int n = wifi_objects.active_access_point.aceesspoint_dir.length();
    char *active_paths = new char[n + 1];
    strcpy(active_paths, wifi_objects.active_access_point.aceesspoint_dir.c_str());

    /* Create a D-Bus proxy to get the object properties from the NM Manager
    * object.  NM_DBUS_* defines are from nm-dbus-interface.h.
    */
    props_proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
                                                G_DBUS_PROXY_FLAGS_NONE,
                                                NULL,
                                                NM_DBUS_SERVICE,
                                                NM_DBUS_PATH,
                                                NM_DBUS_SERVICE,
                                                NULL,
                                                NULL);
    g_assert(props_proxy);

    ret = g_dbus_proxy_call_sync(props_proxy,
                                "DeactivateConnection",
                                g_variant_new("(o)", active_paths),
                                G_DBUS_CALL_FLAGS_NONE,
                                -1,
                                NULL,
                                &error);

    if (ret)
        g_variant_unref(ret);
    else {
        g_dbus_error_strip_remote_error(error);
        g_print("Error disable connection: %s\n", error->message);
        g_clear_error(&error);
    }

    g_object_unref(props_proxy);
}

void NetworkManager::disconnect_WiFi() {
    GDBusProxy *props_proxy;
    GError *     error = NULL;
    GVariant *   ret;

    int n = wifi_objects.wifi_device_path.length();
    char *dev_path = new char[n + 1];
    strcpy(dev_path, wifi_objects.wifi_device_path.c_str());

    /* Create a D-Bus proxy to get the object properties from the NM Manager
    * object.  NM_DBUS_* defines are from nm-dbus-interface.h.
    */
    props_proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
                                                G_DBUS_PROXY_FLAGS_NONE,
                                                NULL,
                                                NM_DBUS_SERVICE,
                                                dev_path,
                                                NM_DBUS_INTERFACE_DEVICE,
                                                NULL,
                                                NULL);
    g_assert(props_proxy);

    ret = g_dbus_proxy_call_sync(props_proxy,
                                "Disconnect",
                                NULL,
                                G_DBUS_CALL_FLAGS_NONE,
                                -1,
                                NULL,
                                &error);

    if (ret)
        g_variant_unref(ret);
    else {
        g_dbus_error_strip_remote_error(error);
        g_print("Error disable connection: %s\n", error->message);
        g_clear_error(&error);
    }

    g_object_unref(props_proxy);
}

void NetworkManager::refresh_wifi() {
    wifi_objects.active_access_point.aceesspoint_dir.clear();
    wifi_objects.active_access_point.ssid.clear();
    wifi_objects.active_access_point.strength = 0;

    wifi_objects.saved_access_points.clear();
    wifi_objects.new_access_points.clear();
    wifi_objects.access_points.clear();

    wifi_init();
}

void NetworkManager::enable_disable_NetworkManager(bool enable) {
    GDBusProxy *props_proxy;
    GError *     error = NULL;
    GVariant *   ret;

    props_proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
                                                G_DBUS_PROXY_FLAGS_NONE,
                                                NULL,
                                                NM_DBUS_SERVICE,
                                                NM_DBUS_PATH,
                                                NM_DBUS_INTERFACE,
                                                NULL,
                                                NULL);
    g_assert(props_proxy);

    ret = g_dbus_proxy_call_sync(props_proxy,
                                "Enable",
                                g_variant_new("(b)", enable),
                                G_DBUS_CALL_FLAGS_NONE,
                                -1,
                                NULL,
                                &error);

    if (ret)
        g_variant_unref(ret);
    else {
        g_dbus_error_strip_remote_error(error);
        g_print("Error Enable/Disable NetworkManager: %s\n", error->message);
        g_clear_error(&error);
    }

    g_object_unref(props_proxy);
}

void NetworkManager::enable_NetworkManager() {
    enable_disable_NetworkManager(true);
}

void NetworkManager::disable_NetworkManager() {
    enable_disable_NetworkManager(false);
}

void NetworkManager::forget_network(const int num_ap) {
    GDBusProxy *props_proxy;
    GError *     error = NULL;
    GVariant *   ret;
    int n; 

    n = wifi_objects.saved_access_points[num_ap].aceesspoint_dir.length();
    char *set_path = new char[n + 1];
    strcpy(set_path, wifi_objects.saved_access_points[num_ap].aceesspoint_dir.c_str());

    props_proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
                                                G_DBUS_PROXY_FLAGS_NONE,
                                                NULL,
                                                NM_DBUS_SERVICE,
                                                set_path,
                                                NM_DBUS_INTERFACE_SETTINGS_CONNECTION,
                                                NULL,
                                                NULL);
    g_assert(props_proxy);

    ret = g_dbus_proxy_call_sync(props_proxy,
                                "Delete",
                                NULL,
                                G_DBUS_CALL_FLAGS_NONE,
                                -1,
                                NULL,
                                &error);

    if (ret) {
        g_variant_unref(ret);
    }
    else {
        g_dbus_error_strip_remote_error(error);
        g_print("Error Read NetworkEnabled: %s\n", error->message);
        g_clear_error(&error);
    }

    g_object_unref(props_proxy);
}

void NetworkManager::update_network_enable() {
    GDBusProxy *props_proxy;
    GError *     error = NULL;
    GVariant *   ret, * value;

    props_proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
                                                G_DBUS_PROXY_FLAGS_NONE,
                                                NULL,
                                                NM_DBUS_SERVICE,
                                                NM_DBUS_PATH,
                                                "org.freedesktop.DBus.Properties",
                                                NULL,
                                                NULL);
    g_assert(props_proxy);

    ret = g_dbus_proxy_call_sync(props_proxy,
                                "Get",
                                g_variant_new("(ss)", NM_DBUS_INTERFACE, "NetworkingEnabled"),
                                G_DBUS_CALL_FLAGS_NONE,
                                -1,
                                NULL,
                                &error);

    if (ret) {
        g_variant_get(ret, "(v)", &value);
        wifi_objects.network_enable = g_variant_get_boolean(value);
        g_variant_unref(ret);
    }
    else {
        g_dbus_error_strip_remote_error(error);
        g_print("Error Read NetworkEnabled: %s\n", error->message);
        g_clear_error(&error);
    }

    g_object_unref(props_proxy);
}

Wifi_Objects NetworkManager::get_wifi_objects() {
    return wifi_objects;
}