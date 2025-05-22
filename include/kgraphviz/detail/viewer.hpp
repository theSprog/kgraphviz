#pragma once
#include <string>
#include <cstdlib>
#include <stdexcept>
#include <sstream>
#include <fstream>

#include "../exceptions.hpp"

namespace kgraphviz {

class Viewer {
  public:
    static void view(const std::string& filepath, bool quiet = false) {
#if defined(_WIN32)
        std::ostringstream cmd;
        cmd << "start \"\" \"" << filepath << "\"";
#elif defined(__APPLE__)
        std::ostringstream cmd;
        cmd << "open \"" << filepath << "\"";
#elif defined(__linux__)
        std::ostringstream cmd;
        cmd << "xdg-open \"" << filepath << "\"";
#else
        throw std::runtime_error("Unsupported platform: cannot view file");
#endif

        if (quiet) {
#if defined(_WIN32)
            // Windows 的 start 无法抑制 stderr；忽略 quiet 参数
#else
            cmd << " 2>/dev/null";
#endif
        }

        int code = std::system(cmd.str().c_str());
        if (code != 0) {
            if (is_running_under_wsl()) {
                if (file_exists(filepath)) {
                    // WSL 特例：xdg-open 实际成功但返回错误码
                    return;
                }else {
                    throw FileNotExistsError(filepath);
                }
            }
            throw std::runtime_error("Failed to open viewer: exit code = " + std::to_string(code));
        }
    }

  private:
    static bool is_running_under_wsl() {
        std::ifstream f("/proc/version");
        std::string version;
        std::getline(f, version);
        return version.find("WSL") != std::string::npos;
    }

    static bool file_exists(const std::string& path) {
        std::ifstream f(path.c_str());
        return f.good();
    }
};

} // namespace kgraphviz
