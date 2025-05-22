#pragma once

#include <stdexcept>
#include <string>
#include <sstream>

namespace kgraphviz {

// Raised when the Graphviz executable is not found
class ExecutableNotFound : public std::runtime_error {
  public:
    explicit ExecutableNotFound(const std::string& msg)
        : std::runtime_error(""), message_("ExecutableNotFound: " + msg) {}

    const char* what() const noexcept override {
        return message_.c_str();
    }

  private:
    std::string message_;
};

// Raised when subprocess exits with non-zero code
class CalledProcessError : public std::runtime_error {
  public:
    CalledProcessError(int return_code,
                       const std::string& cmd,
                       const std::string& output = "",
                       const std::string& error = "")
        : std::runtime_error(""), returncode(return_code), command(cmd), stdout_output(output), stderr_output(error) {
        std::ostringstream oss;
        oss << "CalledProcessError: Command `" << cmd << "` exited with code " << return_code;
        if (! stderr_output.empty()) {
            oss << "\nstderr: " << stderr_output;
        }
        if (! stdout_output.empty()) {
            oss << "\nstdout: " << stdout_output;
        }
        message_ = oss.str();
    }

    const char* what() const noexcept override {
        return message_.c_str();
    }

    int returncode;
    std::string command;
    std::string stdout_output;
    std::string stderr_output;

  private:
    std::string message_;
};

// Raised when required arguments are missing
class RequiredArgumentError : public std::runtime_error {
  public:
    explicit RequiredArgumentError(const std::string& arg_name)
        : std::runtime_error(""), message_("RequiredArgumentError: Missing required argument: " + arg_name) {}

    const char* what() const noexcept override {
        return message_.c_str();
    }

  private:
    std::string message_;
};

// Raised when output file already exists and raise_if_exists == true
class FileExistsError : public std::runtime_error {
  public:
    explicit FileExistsError(const std::string& filepath)
        : std::runtime_error(""), message_("FileExistsError: File already exists: " + filepath) {}

    const char* what() const noexcept override {
        return message_.c_str();
    }

  private:
    std::string message_;
};


class FileNotExistsError : public std::runtime_error {
  public:
    explicit FileNotExistsError(const std::string& filepath)
        : std::runtime_error(""), message_("FileNotExistsError: File not exists: " + filepath) {}

    const char* what() const noexcept override {
        return message_.c_str();
    }

  private:
    std::string message_;
};

} // namespace kgraphviz
