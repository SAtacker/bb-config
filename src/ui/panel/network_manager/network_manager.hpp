#include <stdlib.h>  // for EXIT_SUCCESS
#include <memory>    // for allocator, __shared_ptr_access
#include <string>  // for string, operator+, basic_string, to_string, char_traits
#include <vector>  // for vector, __alloc_traits<>::value_type
#include <cstring>

#include <gio/gio.h>
#include <uuid/uuid.h>
#include <nm-dbus-interface.h>
#include <NetworkManager.h>

struct Accesspoint_Objects {
    std::string aceesspoint_dir;
    std::string ssid;
    int strength;
};

struct Wifi_Objects {
    bool wifi_avaliable;
    bool network_enable;
    std::string wifi_device_path;
    Accesspoint_Objects active_access_point;
    std::vector<Accesspoint_Objects> saved_access_points;
    std::vector<Accesspoint_Objects> new_access_points;
    std::vector<Accesspoint_Objects> access_points;
};

class NetworkManager {
    private:
        Wifi_Objects wifi_objects;

        void wifi_init();
        int get_device_types(char* dev_path);
        int get_accessPoints();
        bool check_wifi_type_activeConnections(const char *);
        void read_access_point_detail(const char *, char *, uint8_t *);
        void read_save_accessPoint_detail(const char*, const int);
        void enable_disable_NetworkManager(const bool);
        void update_wifi_path();
        void update_wireless_accesspoints();
        void update_wifi_activeConnection();
        void update_saved_accesspoints();
        void update_new_accesspoint();
        void update_network_enable();
        
    public:
        NetworkManager();

        void refresh_wifi();
        void disconnect_WiFi();
        void disconnect_active_WiFi();
        void enable_NetworkManager();
        void disable_NetworkManager();
        void forget_network(const int);
        void saved_wireless(const int, const std::string);
        Wifi_Objects get_wifi_objects();
        
};