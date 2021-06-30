#ifndef CONNMAN_HANDLER_HPP
#define CONNMAN_HANDLER_HPP

#include <net/if.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <algorithm>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <thread>

/* ConnMan service provisioning file */
#define CONNMAN_SERVICE_F_PATH "/var/lib/connman"

/* Definition of possible strings in the .config files */
#define CONFIG_KEY_NAME "Name"
#define CONFIG_KEY_DESC "Description"

#define SERVICE_KEY_TYPE "Type"
#define SERVICE_KEY_NAME "Name"
#define SERVICE_KEY_SSID "SSID"
#define SERVICE_KEY_EAP "EAP"
#define SERVICE_KEY_CA_CERT "CACertFile"
#define SERVICE_KEY_CL_CERT "ClientCertFile"
#define SERVICE_KEY_PRV_KEY "PrivateKeyFile"
#define SERVICE_KEY_PRV_KEY_PASS "PrivateKeyPassphrase"
#define SERVICE_KEY_PRV_KEY_PASS_TYPE "PrivateKeyPassphraseType"
#define SERVICE_KEY_IDENTITY "Identity"
#define SERVICE_KEY_ANONYMOUS_IDENTITY "AnonymousIdentity"
#define SERVICE_KEY_SUBJECT_MATCH "SubjectMatch"
#define SERVICE_KEY_ALT_SUBJECT_MATCH "AltSubjectMatch"
#define SERVICE_KEY_DOMAIN_SUFF_MATCH "DomainSuffixMatch"
#define SERVICE_KEY_DOMAIN_MATCH "DomainMatch"
#define SERVICE_KEY_PHASE2 "Phase2"
#define SERVICE_KEY_PASSPHRASE "Passphrase"
#define SERVICE_KEY_SECURITY "Security"
#define SERVICE_KEY_HIDDEN "Hidden"
#define SERVICE_KEY_MDNS "mDNS"

#define SERVICE_KEY_IPv4 "IPv4"
#define SERVICE_KEY_IPv6 "IPv6"
#define SERVICE_KEY_IPv6_PRIVACY "IPv6.Privacy"
#define SERVICE_KEY_MAC "MAC"
#define SERVICE_KEY_DEVICE_NAME "DeviceName"
#define SERVICE_KEY_NAMESERVERS "Nameservers"
#define SERVICE_KEY_SEARCH_DOMAINS "SearchDomains"
#define SERVICE_KEY_TIMESERVERS "Timeservers"
#define SERVICE_KEY_DOMAIN "Domain"

namespace connman_handler {

/* Minimal struct that stores type, name, ssid, pass*/
struct connman_data {
  /* wifi | ethernet */
  std::string type;

  /* SSID */
  std::string name;

  /* Password */
  std::string pass;
};

/*
  Process:
    1. Connecting wifi
      ---- Set fields of `connman_data`
      ---- call connect
      ---- or call disconnect
*/
class connman_h {
 private:
  /* Command Result */
  std::string result;

  /* Current Wifi name */
  std::string service_name;
  std::string active_name;

  /*
   * List of normal - unique names from connmanctl
   * John ----- wifi_<macaddress>_<hash>_managed_psk
   */
  std::unordered_map<std::string, std::string> service_names;

  /*
    Get wifi SSIDs and unique names
    1. Enables wifi
    2. Scans
    3. Parses the output
  */
  void get_service_names();

  /* Service name and its key-value */
  std::unordered_map<std::string, std::unordered_map<std::string, std::string>>
      lookup_table;

  /* If true then change lookup table value */
  bool val_change;

  /* Load Config in map*/
  void parse_file(const char* path = CONNMAN_SERVICE_F_PATH "/connman.conf");

  /* Store the hash table */
  int store_file(const char* path = CONNMAN_SERVICE_F_PATH "/connman.conf");

  /* Comment and store */
  void empty_file(const char* path = CONNMAN_SERVICE_F_PATH "/connman.conf");

  /* Warning: Not thread safe */
  void shell_helper(const char*);

  std::string trim(const std::string& str,
                   const std::string& whitespace = " \t");
  std::string reduce(const std::string& str,
                     const std::string& fill = " ",
                     const std::string& whitespace = " \t");

 public:
  connman_h();
  ~connman_h();
  bool wifi_status();
  void display_service_names();
  int connect_wifi();
  void disconnect_wifi();

  /* Get a vector of names of wifi*/
  std::vector<std::string> get_wifi_names();

  /* Get name of active wifi */
  std::string get_active_name();

  /* Data for wifi */
  struct connman_data data;

  /* Keys for connman services */
  static const std::vector<std::string> service_possible_keys;
};
}  // namespace connman_handler

#endif  // End of include guard: CONNMAN_HANDLER_HPP