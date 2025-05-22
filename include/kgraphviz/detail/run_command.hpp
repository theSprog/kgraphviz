#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <vector>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <cerrno>
#include <cstring>

namespace kgraphviz {
namespace {
template <typename Derived>
struct ByteSink {
    void append(const uint8_t* data, size_t len) {
        static_cast<Derived*>(this)->append_impl(data, len);
    }

    void clear() {
        static_cast<Derived*>(this)->clear_impl();
    }
};

struct VectorSink : ByteSink<VectorSink> {
    std::vector<uint8_t>& out;

    explicit VectorSink(std::vector<uint8_t>& out_) : out(out_) {}

    void append_impl(const uint8_t* data, size_t len) {
        out.insert(out.end(), data, data + len);
    }

    void clear_impl() {
        out.clear();
    }
};

struct StringSink : ByteSink<StringSink> {
    std::string& out;

    explicit StringSink(std::string& out_) : out(out_) {}

    void append_impl(const uint8_t* data, size_t len) {
        out.append(reinterpret_cast<const char*>(data), len);
    }

    void clear_impl() {
        out.clear();
    }
};

template <typename StdoutSink, typename StderrSink>
inline int run_command_sink(const std::string& cmd,
                            StdoutSink& stdout_sink,
                            StderrSink& stderr_sink,
                            const std::string* stdin_data = nullptr) {
    int stdin_pipe[2], stdout_pipe[2], stderr_pipe[2];
    if (pipe(stdout_pipe) != 0 || pipe(stderr_pipe) != 0 || pipe(stdin_pipe) != 0) {
        return -1;
    }

    pid_t pid = fork();
    if (pid < 0) { // fork 失败, 清理返回
        close(stdout_pipe[0]);
        close(stdout_pipe[1]);
        close(stderr_pipe[0]);
        close(stderr_pipe[1]);
        close(stdin_pipe[0]);
        close(stdin_pipe[1]);
        return -2;
    }

    if (pid == 0) {
        // 子进程：重定向 stdin/stdout/stderr
        dup2(stdin_pipe[0], STDIN_FILENO);
        dup2(stdout_pipe[1], STDOUT_FILENO);
        dup2(stderr_pipe[1], STDERR_FILENO);

        close(stdin_pipe[0]);
        close(stdin_pipe[1]);
        close(stdout_pipe[0]);
        close(stdout_pipe[1]);
        close(stderr_pipe[0]);
        close(stderr_pipe[1]);

        execl("/bin/sh", "sh", "-c", cmd.c_str(), (char*)nullptr);
        _exit(127);
    }

    // 父进程
    close(stdin_pipe[0]);
    close(stdout_pipe[1]);
    close(stderr_pipe[1]);

    // 写入 stdin 数据（如 DOT 字符串）
    if (stdin_data && ! stdin_data->empty()) {
        ssize_t n = write(stdin_pipe[1], stdin_data->data(), stdin_data->size());
        if (n < 0) {
            close(stdin_pipe[1]);
            close(stdout_pipe[0]);
            close(stderr_pipe[0]);
            return -5;
        }
    }
    close(stdin_pipe[1]); // 重要：关闭 write 端，防止子进程 hang 住等待 EOF

    stdout_sink.clear();
    stderr_sink.clear();

    char buf[4096];
    ssize_t n;

    while ((n = read(stdout_pipe[0], buf, sizeof(buf))) > 0) {
        stdout_sink.append(reinterpret_cast<const uint8_t*>(buf), n);
    }
    close(stdout_pipe[0]);

    while ((n = read(stderr_pipe[0], buf, sizeof(buf))) > 0) {
        stderr_sink.append(reinterpret_cast<const uint8_t*>(buf), n);
    }
    close(stderr_pipe[0]);

    int status;
    if (waitpid(pid, &status, 0) < 0) return -3;
    return WIFEXITED(status) ? WEXITSTATUS(status) : -4;
}
} // namespace

inline int run_command(const std::string& cmd, std::vector<uint8_t>& out_bin, std::vector<uint8_t>& err_bin) {
    VectorSink out_sink(out_bin), err_sink(err_bin);
    return run_command_sink(cmd, out_sink, err_sink);
}

inline int run_command(const std::string& cmd, std::vector<uint8_t>& out_bin, std::string& err_text) {
    VectorSink out_sink(out_bin);
    StringSink err_sink(err_text);
    return run_command_sink(cmd, out_sink, err_sink);
}

inline int run_command(const std::string& cmd, std::string& out_text, std::string& err_text) {
    StringSink out_sink(out_text), err_sink(err_text);
    return run_command_sink(cmd, out_sink, err_sink);
}

inline int run_command_with_stdin(const std::string& stdin_data,
                                  const std::string& cmd,
                                  std::vector<uint8_t>& stdout_output,
                                  std::string& stderr_output) {
    VectorSink out(stdout_output);
    StringSink err(stderr_output);
    return run_command_sink(cmd, out, err, &stdin_data);
}


} // namespace kgraphviz
