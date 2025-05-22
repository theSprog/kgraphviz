#pragma once
#include <iostream>
#include <string>
#include <fstream>
#include <stdexcept>
#include "options.hpp"

#include "detail/render.hpp"
#include "detail/viewer.hpp"
#include "detail/tmpfile.hpp"

namespace kgraphviz {

class Source {
  public:
    Source(const std::string& dot_code, const SourceOptions& options = SourceOptions())
        : dot_code_(dot_code), source_options_(options) {}

    std::string source_filepath() const {
        if (source_options_.filename.empty()) {
            throw RequiredArgumentError("save() require filename arugument");
        }
        return source_options_.directory.empty() ? source_options_.filename :
                                                   (source_options_.directory + "/" + source_options_.filename);
    }

    void save() const {
        std::ofstream out(source_filepath().c_str(), std::ios::binary);
        if (! out) throw std::runtime_error("Failed to open file: " + source_filepath());
        out << dot_code_;
    }

    void render(const std::string& out_file, const RenderOptions& render_opts = RenderOptions()) const {
        Renderer::render_from_string(dot_code_, out_file, render_opts);
    }

    std::vector<uint8_t> render_to_memory(const RenderOptions& render_opts = RenderOptions()) const {
        return Renderer::render_from_string_to_memory(dot_code_, render_opts);
    }

    void view(RenderOptions render_opts = RenderOptions()) const {
        if (render_opts.format.empty()) render_opts.format = DefaultFormat;
        std::string output_path = TmpFile::generate_path(render_opts.format);
        render(output_path, render_opts);
        Viewer::view(output_path, render_opts.quiet);
    }

  private:
    std::string dot_code_;
    SourceOptions source_options_;
};


} // namespace kgraphviz
