#include "include/platform.h"

#include <cerrno>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <system_error>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <shellapi.h>
#else
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>
extern char **environ;
#endif

namespace xxcc {

#ifdef _WIN32
namespace {

std::wstring wide(const std::string &text)
{
    if (text.empty()) return {};
    const int length = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
                                         text.data(), static_cast<int>(text.size()), nullptr, 0);
    if (!length) throw std::runtime_error("invalid UTF-8 argument");
    std::wstring result(length, L'\0');
    MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text.data(),
                        static_cast<int>(text.size()), &result[0], length);
    return result;
}

std::string utf8(const std::wstring &text)
{
    if (text.empty()) return {};
    const int length = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, text.data(),
                                         static_cast<int>(text.size()), nullptr, 0, nullptr, nullptr);
    if (!length) throw std::runtime_error("invalid Unicode path");
    std::string result(length, '\0');
    WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, text.data(),
                        static_cast<int>(text.size()), &result[0], length, nullptr, nullptr);
    return result;
}

// CreateProcess takes a command line, not argv. Escape according to the CRT
// rules, including backslashes before quotes and the closing quote.
std::wstring quote_windows(const std::string &argument)
{
    const std::wstring value = wide(argument);
    std::wstring result = L"\"";
    std::size_t slashes = 0;
    for (wchar_t ch : value) {
        if (ch == L'\\') {
            ++slashes;
        } else {
            result.append(ch == L'"' ? slashes * 2 + 1 : slashes, L'\\');
            result += ch;
            slashes = 0;
        }
    }
    result.append(slashes * 2, L'\\');
    result += L'"';
    return result;
}

class Handle {
public:
    explicit Handle(HANDLE value) : value_(value) {}
    ~Handle() { CloseHandle(value_); }
    Handle(const Handle &) = delete;
    Handle &operator=(const Handle &) = delete;
    HANDLE get() const { return value_; }
private:
    HANDLE value_;
};

} // namespace
#endif

std::vector<std::string> arguments(int argc, char **argv)
{
#ifdef _WIN32
    (void)argc;
    (void)argv;
    int count = 0;
    LPWSTR *values = CommandLineToArgvW(GetCommandLineW(), &count);
    if (!values) throw std::runtime_error("cannot read command line");
    std::vector<std::string> result;
    try {
        for (int i = 0; i < count; ++i) result.push_back(utf8(values[i]));
    } catch (...) {
        LocalFree(values);
        throw;
    }
    LocalFree(values);
    return result;
#else
    return {argv, argv + argc};
#endif
}

std::string environment(const char *name)
{
#ifdef _WIN32
    const std::wstring key = wide(name);
    const DWORD length = GetEnvironmentVariableW(key.c_str(), nullptr, 0);
    if (!length) return {};
    std::wstring value(length, L'\0');
    const DWORD copied = GetEnvironmentVariableW(key.c_str(), &value[0], length);
    if (!copied || copied >= length) throw std::runtime_error("cannot read environment variable");
    value.resize(copied);
    return utf8(value);
#else
    const char *value = std::getenv(name);
    return value ? value : "";
#endif
}

fs::path executable_path(const std::string &argv0)
{
#ifdef _WIN32
    (void)argv0;
    std::vector<wchar_t> buffer(512);
    for (;;) {
        const DWORD copied = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
        if (!copied) throw std::runtime_error("cannot locate executable");
        if (copied < buffer.size()) return fs::path(std::wstring(buffer.data(), copied));
        buffer.resize(buffer.size() * 2);
    }
#else
    std::error_code error;
    fs::path result = fs::read_symlink("/proc/self/exe", error);
    if (!error) return result;
    fs::path supplied = fs::u8path(argv0);
    if (supplied.has_parent_path()) return fs::absolute(supplied);
    const std::string paths = environment("PATH");
    std::size_t start = 0;
    do {
        const std::size_t end = paths.find(':', start);
        const std::string part = paths.substr(start, end == std::string::npos ? end : end - start);
        const fs::path candidate = fs::u8path(part.empty() ? "." : part) / supplied;
        if (access(candidate.c_str(), X_OK) == 0) return fs::absolute(candidate);
        if (end == std::string::npos) break;
        start = end + 1;
    } while (start <= paths.size());
    throw std::runtime_error("cannot locate executable; use --xxcc-sdk=PATH");
#endif
}

int run_process(const std::vector<std::string> &command, bool verbose)
{
    if (command.empty()) throw std::runtime_error("empty command");
    if (verbose) {
        std::cerr << "xxcc:";
        for (const std::string &arg : command) {
            std::cerr << " [";
            for (char ch : arg) {
                if (ch == '\n') std::cerr << "\\n";
                else if (ch == '\r') std::cerr << "\\r";
                else std::cerr << ch;
            }
            std::cerr << ']';
        }
        std::cerr << '\n';
    }
#ifdef _WIN32
    std::wstring line;
    for (const std::string &arg : command) {
        if (!line.empty()) line += L' ';
        line += quote_windows(arg);
    }
    STARTUPINFOW startup = {};
    startup.cb = sizeof(startup);
    PROCESS_INFORMATION process = {};
    if (!CreateProcessW(nullptr, &line[0], nullptr, nullptr, TRUE, 0, nullptr, nullptr,
                        &startup, &process)) {
        const DWORD error = GetLastError();
        std::cerr << "xxcc: cannot start '" << command[0] << "': "
                  << std::system_category().message(error) << '\n';
        return error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND ? 127 : 126;
    }
    Handle thread(process.hThread);
    Handle child(process.hProcess);
    if (WaitForSingleObject(child.get(), INFINITE) != WAIT_OBJECT_0)
        throw std::runtime_error("cannot wait for child process");
    DWORD code = 0;
    if (!GetExitCodeProcess(child.get(), &code))
        throw std::runtime_error("cannot read child exit status");
    return static_cast<int>(code);
#else
    std::vector<char *> argv;
    for (const std::string &arg : command) argv.push_back(const_cast<char *>(arg.c_str()));
    argv.push_back(nullptr);
    pid_t child = 0;
    const int error = posix_spawnp(&child, argv[0], nullptr, nullptr, argv.data(), environ);
    if (error) {
        std::cerr << "xxcc: cannot start '" << command[0] << "': " << std::strerror(error) << '\n';
        return error == ENOENT ? 127 : 126;
    }
    int status = 0;
    while (waitpid(child, &status, 0) == -1) {
        if (errno != EINTR) throw std::system_error(errno, std::generic_category(), "waitpid");
    }
    if (WIFEXITED(status)) return WEXITSTATUS(status);
    if (WIFSIGNALED(status)) return 128 + WTERMSIG(status);
    return 1;
#endif
}

void publish_file(const fs::path &source, const fs::path &destination)
{
    if (!fs::is_regular_file(source))
        throw std::runtime_error("backend succeeded without creating '" + source.u8string() + "'");
#ifdef _WIN32
    if (!MoveFileExW(source.c_str(), destination.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
        throw std::system_error(GetLastError(), std::system_category(), "cannot publish output");
#else
    fs::rename(source, destination);
#endif
}

TemporaryDirectory::TemporaryDirectory(const fs::path &parent, bool keep) : keep_(keep)
{
#ifdef _WIN32
    const auto pid = GetCurrentProcessId();
#else
    const auto pid = getpid();
#endif
    const auto tick = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    for (unsigned attempt = 0; attempt < 100; ++attempt) {
        const fs::path candidate = parent / (".xxcc-" + std::to_string(pid) + "-" +
                                             std::to_string(tick) + "-" + std::to_string(attempt));
        std::error_code error;
        if (fs::create_directory(candidate, error)) {
            path_ = candidate;
            return;
        }
        if (error && error != std::errc::file_exists)
            throw fs::filesystem_error("cannot create temporary directory", candidate, error);
    }
    throw std::runtime_error("cannot allocate a unique temporary directory");
}

TemporaryDirectory::~TemporaryDirectory()
{
    if (keep_) return;
    std::error_code error;
    fs::remove_all(path_, error);
    if (error) std::cerr << "xxcc: warning: cannot clean '" << path_.u8string() << "': " << error.message() << '\n';
}

} // namespace xxcc
