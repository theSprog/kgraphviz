#pragma once
#include <cstdint>
#include <string>
#include <string>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <vector>
#include "run_command.hpp"

#include "../exceptions.hpp"
#include "../options.hpp"

namespace kgraphviz {

class Renderer {
  public:
    static void
    render(const std::string& input_file, const std::string& output_file, RenderOptions options = RenderOptions()) {
        validate_options(options, input_file, output_file);

        std::string fmt = deduce_format(output_file, options);

        std::ostringstream cmd = build_command(input_file, output_file, options, /*to_stdout=*/false);
        std::string full_cmd = cmd.str();

        std::string stdout_output, stderr_output;
        int exit_code = run_command(full_cmd, stdout_output, stderr_output);
        if (exit_code != 0) {
            throw CalledProcessError(exit_code, full_cmd, stdout_output, options.quiet ? "" : stderr_output);
        }
    }

    // render_to_memory 必须在 options 中指定 format, 否则不知道推断为什么格式
    static std::vector<uint8_t> render_to_memory(const std::string& input_file,
                                                 const RenderOptions& options = RenderOptions()) {
        validate_options(options, input_file, /*output_file*/ "");

        std::ostringstream cmd = build_command(input_file, "", options, /*to_stdout=*/true);
        std::string full_cmd = cmd.str();

        std::vector<uint8_t> binary_output;
        std::string stderr_output;
        int exit_code = run_command(full_cmd, binary_output, stderr_output);
        if (exit_code != 0) {
            throw CalledProcessError(exit_code, full_cmd, "<ignored>", options.quiet ? "" : stderr_output);
        }

        return binary_output;
    }

    // 渲染字符串为文件（使用 stdin, 避免创建 .gv 文件）
    static void render_from_string(const std::string& dot_source,
                                   const std::string& output_file,
                                   RenderOptions options = RenderOptions()) {
        validate_options(options, /*input_file*/ "", output_file);

        if (output_file.empty()) {
            throw RequiredArgumentError("output_file (required)");
        }

        std::string fmt = deduce_format(output_file, options);

        std::ostringstream cmd = build_command(
            /*input_file*/ "",
            output_file,
            options,
            /*to_stdout=*/false,
            /*use_stdin=*/true);


        std::vector<uint8_t> ignored;
        std::string stderr_output;
        int code = run_command_with_stdin(dot_source, cmd.str(), ignored, stderr_output);
        if (code != 0) {
            throw CalledProcessError(code, cmd.str(), "<ignored>", options.quiet ? "" : stderr_output);
        }
    }

    // 渲染字符串为内存图像（无需任何临时文件）
    static std::vector<uint8_t> render_from_string_to_memory(const std::string& dot_source,
                                                             const RenderOptions& options = RenderOptions()) {
        validate_options(options, /*input_file*/ "", /*output_file*/ "");

        std::ostringstream cmd = build_command(
            /*input_file*/ "",
            /*output_file*/ "",
            options,
            /*to_stdout=*/true,
            /*use_stdin=*/true);

        std::vector<uint8_t> out;
        std::string stderr_output;
        int code = run_command_with_stdin(dot_source, cmd.str(), out, stderr_output);
        if (code != 0) {
            throw CalledProcessError(code, cmd.str(), "<ignored>", options.quiet ? "" : stderr_output);
        }

        return out;
    }

  private:
    static void
    validate_options(const RenderOptions& options, const std::string& input_file, const std::string& output_file) {
        if (! options.formatter.empty() && options.renderer.empty()) {
            throw RequiredArgumentError("renderer (required by formatter)");
        }
        if (! is_executable_available(options.engine)) {
            throw ExecutableNotFound(options.engine);
        }
        if (input_file == output_file && input_file != "" && ! options.overwrite_filepath) {
            throw RequiredArgumentError("overwrite_filepath=true required when input_file == output_file");
        }
        if (options.raise_if_result_exists) {
            std::ifstream check(output_file.c_str());
            if (check.good()) {
                throw FileExistsError(output_file);
            }
        }
    }

    static std::ostringstream build_command(const std::string& input_file,
                                            const std::string& output_file,
                                            const RenderOptions& options,
                                            bool to_stdout,
                                            bool use_stdin = false) {
        std::ostringstream cmd;
        cmd << options.engine;

        if (options.format.empty()) {
            throw RequiredArgumentError("format");
        }

        cmd << " -T" << options.format;

        if (! options.renderer.empty()) {
            cmd << ":" << options.renderer;
            if (! options.formatter.empty()) {
                cmd << ":" << options.formatter;
            }
        }

        if (options.neato_no_op) {
            cmd << " -n";
        }

        // 输入文件
        if (! use_stdin) {
            cmd << " \"" << input_file << "\"";
        }

        // 输出到文件或 stdout
        if (to_stdout) {
            // 什么都不做, 默认输出到 stdout
        } else if (! output_file.empty()) {
            cmd << " -o \"" << output_file << "\"";
        }

        return cmd;
    }

    static bool is_executable_available(const std::string& exe) {
#if defined(_WIN32)
        std::string test_cmd = "where " + exe + " >nul 2>&1";
#else
        std::string test_cmd = "command -v " + exe + " >/dev/null 2>&1";
#endif
        return std::system(test_cmd.c_str()) == 0;
    }

    static std::string get_format_from_filename(const std::string& filename) {
        auto pos = filename.rfind('.');
        if (pos != std::string::npos && pos + 1 < filename.size()) {
            return filename.substr(pos + 1); // e.g. "svg"
        }
        return "";
    }

    static std::string deduce_format(const std::string& filename, RenderOptions& options) {
        std::string fmt = options.format;
        if (fmt.empty()) {
            // 隐式推断类型
            fmt = get_format_from_filename(filename);
            if (fmt.empty()) {
                throw RequiredArgumentError("format must be set either via options or output filename");
            } else {
                options.set_format(fmt);
            }
        }
        return fmt;
    }
};

} // namespace kgraphviz
