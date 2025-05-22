#pragma once
#include <cstring>
#include <string>
#include <fstream>
#include <cstdio>
#include <stdexcept>
#include <cstdlib>
#include <unistd.h>

namespace kgraphviz {

class TmpFile {
  public:
    static std::string create(const std::string& suffix = "tmp") {
        std::string path = generate_path(suffix);
        std::ofstream ofs(path);
        if (! ofs) {
            throw std::runtime_error("TempFile: Failed to open file: " + path);
        }
        ofs.close();
        return path;
    }

    // 创建带后缀的临时文件并写入内容，返回文件路径
    static std::string create_with_content(const std::string& content, const std::string& suffix = "tmp") {
        std::string path = generate_path(suffix);
        std::ofstream ofs(path);
        if (! ofs) {
            throw std::runtime_error("TempFile: Failed to open file: " + path);
        }
        ofs << content;
        ofs.close();
        return path;
    }

    static std::string generate_path(const std::string& suffix = "tmp") {
#ifdef _WIN32
        char tmp_path[MAX_PATH];
        if (! GetTempPathA(MAX_PATH, tmp_path)) {
            throw std::runtime_error("TempFile: Failed to get temp path.");
        }
        char tmp_file[MAX_PATH];
        if (! GetTempFileNameA(tmp_path, "gv", 0, tmp_file)) {
            throw std::runtime_error("TempFile: Failed to get temp file name.");
        }
        std::string result(tmp_file);
        result += suffix;
        return result;
#else
        std::string tmpl = "/tmp/kgraphviz_XXXXXX";
        char buf[tmpl.size() + 1];
        std::strcpy(buf, tmpl.c_str());
        int fd = mkstemp(buf);
        if (fd == -1) {
            throw std::runtime_error("TempFile: mkstemp failed.");
        }
        close(fd); // We will reopen with ofstream
        std::string result(buf);
        result += ("." + suffix);
        std::rename(buf, result.c_str());
        return result;
#endif
    }
};

} // namespace kgraphviz
