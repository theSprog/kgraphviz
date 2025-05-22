#pragma once
#include <string>

namespace kgraphviz {

const static std::string DefaultFormat = "svg";

struct RenderOptions {
    std::string engine = "dot"; // layout engine
    std::string format = "";    // 默认可以从 output filename 中推断出来

    // 保持 renderer / formatter 的默认值为空字符串 "" 是合理且安全的设计。
    std::string renderer;  // e.g., "cairo"
    std::string formatter; // e.g., "gd"

    bool neato_no_op = false;
    bool quiet = false;
    bool raise_if_result_exists = false;
    bool overwrite_filepath = false;

    RenderOptions& set_engine(const std::string& eng) {
        engine = eng;
        return *this;
    }

    RenderOptions& set_format(const std::string& fmt) {
        format = fmt;
        return *this;
    }

    RenderOptions& set_renderer(const std::string& r) {
        renderer = r;
        return *this;
    }

    RenderOptions& set_formatter(const std::string& f) {
        formatter = f;
        return *this;
    }

    RenderOptions& set_neato_no_op(bool flag) {
        neato_no_op = flag;
        return *this;
    }

    RenderOptions& set_quiet(bool flag) {
        quiet = flag;
        return *this;
    }

    RenderOptions& set_raise_if_result_exists(bool flag) {
        raise_if_result_exists = flag;
        return *this;
    }

    RenderOptions& set_overwrite_filepath(bool flag) {
        overwrite_filepath = flag;
        return *this;
    }
};

struct SourceOptions {
    std::string filename = "default.gv"; // e.g., default.gv
    std::string directory = "";          // e.g., tmp/
    std::string encoding = "utf-8";      // not yet used, reserved

    SourceOptions& set_filename(const std::string& f) {
        filename = f;
        return *this;
    }

    SourceOptions& set_directory(const std::string& dir) {
        directory = dir;
        return *this;
    }

    SourceOptions& set_encoding(const std::string& enc) {
        encoding = enc;
        return *this;
    }
};
} // namespace kgraphviz
