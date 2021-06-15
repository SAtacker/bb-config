#include "xdg_handler.hpp"

using namespace xdg_h;

static constexpr const char HOME[]{"HOME"};
static constexpr const char XDG_DATA_HOME_SUFFIX[]{"/.local/share"};
static constexpr const char XDG_CONFIG_HOME_SUFFIX[]{"/.config"};
static constexpr const char XDG_CACHE_HOME_SUFFIX[]{"/.cache"};
static constexpr const char XDG_DATA_DIRS_DEFAULT[]{
    "/usr/local/share/:/usr/share/"};
static constexpr const char XDG_CONFIG_DIRS_DEFAULT[]{"/etc/xdg"};

namespace env {
static std::string get(const std::string& name,
                            const std::string& default_value);
static std::string get(const std::string& name);
}  // namespace env

namespace string_utils {
template <typename Out>
static void split(const std::string& s,
                  const std::string& delimiter,
                  Out result) {
  size_t start = 0;
  size_t end = s.find(delimiter);

  while (end != std::string::npos) {
    *(result++) = s.substr(start, end - start);
    start = end + delimiter.length();
    end = s.find(delimiter, start);
  }

  *(result++) = s.substr(start, end);
}

static std::vector<std::string> split(const std::string& s,
                                      const std::string& delimiter) {
  std::vector<std::string> tokens;
  split(s, delimiter, std::back_inserter(tokens));

  return tokens;
}
}  // namespace string_utils

static bool is_absolute_path(const std::string& path);
static std::vector<std::string> remove_relative_paths(
    const std::vector<std::string>& paths);

bool is_absolute_path(const std::string& path) {
  return (!path.empty() && path[0] == '/');
}

std::vector<std::string> remove_relative_paths(
    const std::vector<std::string>& paths) {
  std::vector<std::string> absolute_paths;

  for (const auto& p : paths)
    if (is_absolute_path(p))
      absolute_paths.push_back(p);

  return absolute_paths;
}

std::string env::get(const std::string& name) {
  if (auto value = std::getenv(name.c_str()))
    return value;

  throw std::runtime_error(name + (": cannot be found"));
}

std::string env::get(const std::string& name,
                     const std::string& default_value) {
  if (auto value = std::getenv(name.c_str()))
    return value;
  return default_value;
}

std::string data::home() {
  auto path = env::get(XDG_DATA_HOME, "");

  if (!is_absolute_path(path)) {
    path = env::get(HOME) + XDG_DATA_HOME_SUFFIX;
  }

  if (!is_absolute_path(path))
    path = {};

  return path;
}

std::vector<std::string> data::dirs() {
  auto paths = env::get(XDG_DATA_DIRS, "");

  if (paths.empty()) {
    paths = XDG_DATA_DIRS_DEFAULT;
  }

  return remove_relative_paths(string_utils::split(paths, ":"));
}

std::string config::home() {
  auto path = env::get(XDG_CONFIG_HOME, "");

  if (!is_absolute_path(path)) {
    path = env::get(HOME) + XDG_CONFIG_HOME_SUFFIX;
  }

  if (!is_absolute_path(path))
    path = {};

  return path;
}

std::vector<std::string> config::dirs() {
  auto paths = env::get(XDG_CONFIG_DIRS, "");

  if (paths.empty()) {
    paths = XDG_CONFIG_DIRS_DEFAULT;
  }

  return remove_relative_paths(string_utils::split(paths, ":"));
}

std::string cache::home() {
  auto path = env::get(XDG_CACHE_HOME, "");

  if (!is_absolute_path(path)) {
    path = env::get(HOME) + XDG_CACHE_HOME_SUFFIX;
  }

  if (!is_absolute_path(path))
    path = {};

  return path;
}

std::string runtime::dir() {
  auto path = env::get(XDG_RUNTIME_DIR, "");

  if (!is_absolute_path(path))
    path = {};

  return path;
}