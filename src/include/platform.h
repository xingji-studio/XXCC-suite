#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace xxcc {

namespace fs = std::filesystem;

std::vector<std::string> arguments(int argc, char **argv);
std::string environment(const char *name);
fs::path executable_path(const std::string &argv0);
int run_process(const std::vector<std::string> &command, bool verbose);
void publish_file(const fs::path &source, const fs::path &destination);

class TemporaryDirectory {
public:
    explicit TemporaryDirectory(const fs::path &parent, bool keep = false);
    ~TemporaryDirectory();
    TemporaryDirectory(const TemporaryDirectory &) = delete;
    TemporaryDirectory &operator=(const TemporaryDirectory &) = delete;

    const fs::path &path() const { return path_; }

private:
    fs::path path_;
    bool keep_;
};

} // namespace xxcc
