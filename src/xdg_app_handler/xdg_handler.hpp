#ifndef XDG_HANDLER_HPP
#define XDG_HANDLER_HPP

#include <cstdlib>
#include <stdexcept>
#include <string>
#include <vector>

namespace xdg_h {

static const std::string XDG_DATA_HOME{"XDG_DATA_HOME"};
static const std::string XDG_DATA_DIRS{"XDG_DATA_DIRS"};
static const std::string XDG_CONFIG_HOME{"XDG_CONFIG_HOME"};
static const std::string XDG_CONFIG_DIRS{"XDG_CONFIG_DIRS"};
static const std::string XDG_CACHE_HOME{"XDG_CACHE_HOME"};
static const std::string XDG_RUNTIME_DIR{"XDG_RUNTIME_DIR"};

namespace data {
std::string home();
std::vector<std::string> dirs();
}  // namespace data

namespace config {
std::string home();
std::vector<std::string> dirs();
}  // namespace config

namespace cache {
std::string home();
}

namespace runtime {
std::string dir();
}

}  // namespace xdg_h

#endif  // End of include guard: XDG_HANDLER_HPP