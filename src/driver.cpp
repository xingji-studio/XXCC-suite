#include "include/driver.h"
#include "include/platform.h"

#include <algorithm>
#include <iostream>
#include <memory>
#include <set>
#include <stdexcept>

namespace xxcc {
namespace {

enum class Stage { Link, Object, Assembly, Preprocess };

struct Input {
    std::string value;
    bool file;
    std::string language;
};

struct Options {
    Stage stage = Stage::Link;
    std::string output;
    std::string sdk;
    std::vector<std::string> compile_args;
    std::vector<Input> link_items;
    bool verbose = false;
    bool keep = false;
    bool dependencies = false;
    bool dependency_file = false;
    bool dependency_target = false;
};

bool starts_with(const std::string &text, const std::string &prefix)
{
    return text.compare(0, prefix.size(), prefix) == 0;
}

std::string operand(const std::vector<std::string> &args, std::size_t &index)
{
    if (index + 1 == args.size() || args[index + 1].empty())
        throw std::runtime_error("missing argument after '" + args[index] + "'");
    return args[++index];
}

Options parse(const std::vector<std::string> &args)
{
    const std::set<std::string> valued = {
        "-I", "-isystem", "-iquote", "-idirafter", "-include", "-imacros",
        "-D", "-U", "-isysroot", "--sysroot", "-target", "--target", "-arch",
        "-B", "-MF", "-MT", "-MQ", "-MJ", "-Xclang", "-Xassembler", "-mllvm",
        "-iprefix", "-iwithprefix", "-iwithprefixbefore", "-resource-dir"
    };
    Options result;
    bool positional = false;
    std::string language;
    for (std::size_t i = 1; i < args.size(); ++i) {
        const std::string &arg = args[i];
        if (positional || arg.empty() || arg[0] != '-') {
            if (!positional && starts_with(arg, "@"))
                throw std::runtime_error("response files are not supported in this release");
            result.link_items.push_back({arg, true, language});
        } else if (arg == "--") {
            positional = true;
        } else if (arg == "-o" || (starts_with(arg, "-o") && arg.size() > 2)) {
            if (!result.output.empty()) throw std::runtime_error("output specified more than once");
            result.output = arg == "-o" ? operand(args, i) : arg.substr(2);
        } else if (arg == "--xxcc-sdk" || starts_with(arg, "--xxcc-sdk=")) {
            result.sdk = arg == "--xxcc-sdk" ? operand(args, i) : arg.substr(11);
            if (result.sdk.empty()) throw std::runtime_error("empty SDK path");
        } else if (arg == "--xxcc-save-temps") {
            result.keep = true;
        } else if (arg == "-v" || arg == "--xxcc-verbose") {
            result.verbose = true;
        } else if (starts_with(arg, "--xxcc-")) {
            throw std::runtime_error("unknown XXCC option '" + arg + "'");
        } else if (arg == "-E" || arg == "-M" || arg == "-MM") {
            result.stage = Stage::Preprocess;
            if (arg != "-E") result.compile_args.push_back(arg);
        } else if (arg == "-S") {
            if (result.stage != Stage::Preprocess) result.stage = Stage::Assembly;
        } else if (arg == "-c") {
            if (result.stage == Stage::Link) result.stage = Stage::Object;
        } else if (arg == "-x" || (starts_with(arg, "-x") && arg.size() > 2)) {
            language = arg == "-x" ? operand(args, i) : arg.substr(2);
            if (language == "none") language.clear();
        } else if (arg == "-l" || arg == "-L" || arg == "-Xlinker") {
            const std::string value = operand(args, i);
            if (arg != "-Xlinker") result.link_items.push_back({arg, false, {}});
            result.link_items.push_back({value, false, {}});
        } else if (starts_with(arg, "-l") || starts_with(arg, "-L")) {
            result.link_items.push_back({arg, false, {}});
        } else if (starts_with(arg, "-Wl,")) {
            std::size_t start = 4;
            do {
                const std::size_t end = arg.find(',', start);
                const std::string value = arg.substr(start, end == std::string::npos ? end : end - start);
                if (value.empty()) throw std::runtime_error("empty option in '" + arg + "'");
                result.link_items.push_back({value, false, {}});
                if (end == std::string::npos) break;
                start = end + 1;
            } while (start <= arg.size());
        } else {
            result.compile_args.push_back(arg);
            if (arg == "-MD" || arg == "-MMD") result.dependencies = true;
            if (starts_with(arg, "-MF")) result.dependency_file = true;
            if (starts_with(arg, "-MT") || starts_with(arg, "-MQ")) result.dependency_target = true;
            if (valued.count(arg)) result.compile_args.push_back(operand(args, i));
        }
    }
    return result;
}

bool is_source(const Input &item)
{
    const std::string extension = fs::u8path(item.value).extension().u8string();
    return item.file && (!item.language.empty() || (extension != ".o" && extension != ".a"));
}

fs::path find_sdk(const Options &options, const std::string &argv0, const Profile &profile)
{
    const std::string supplied = options.sdk.empty() ? environment("XXCC_SDK_ROOT") : options.sdk;
    if (!supplied.empty()) {
        const fs::path root = fs::absolute(fs::u8path(supplied));
        if (!fs::is_directory(root / "include"))
            throw std::runtime_error("SDK include directory not found: '" + (root / "include").u8string() + "'");
        if (options.stage == Stage::Link && !fs::is_directory(root / "lib" / "x86_64"))
            throw std::runtime_error("SDK runtime directory not found: '" + (root / "lib" / "x86_64").u8string() + "'");
        return root;
    }
    fs::path candidate = executable_path(argv0).parent_path();
    // Support both an installed SDK beside the executable and release/{platform}.
    for (unsigned depth = 0; depth < 4; ++depth) {
        if (fs::is_directory(candidate / "include") &&
            (options.stage != Stage::Link || fs::is_directory(candidate / "lib" / "x86_64")))
            return candidate;
        const fs::path parent = candidate.parent_path();
        if (parent == candidate) break;
        candidate = parent;
    }
    throw std::runtime_error("cannot find SDK beside the executable; set --xxcc-sdk=PATH or XXCC_SDK_ROOT");
}

void require_file(const fs::path &path)
{
    if (!fs::is_regular_file(path))
        throw std::runtime_error("required file not found: '" + path.u8string() + "'");
}

struct Runtime {
    std::vector<std::string> objects;
    std::vector<std::string> libraries;
    fs::path startup;
};

Runtime load_runtime(const fs::path &sdk, const Profile &profile)
{
    // XACT CRT layout: include/ + lib/x86_64/{startup objects, archives}.
    const fs::path directory = sdk / "lib" / "x86_64";
    Runtime result;
    result.startup = directory / (profile.tui ? "constart.o" : "start.o");
    require_file(result.startup);
    require_file(directory / "crt0.o");
    require_file(directory / "libxact.a");
    require_file(directory / "liballoc.a");
    if (!profile.tui) require_file(directory / "libxgui.a");
    result.objects.push_back((directory / "crt0.o").generic_u8string());
    // Archives follow all user inputs so GNU ld can resolve their dependencies.
    // libxact is core/TUI; libxgui is only pulled by GUI variants.
    result.libraries.push_back((directory / "libxact.a").generic_u8string());
    if (!profile.tui) result.libraries.push_back((directory / "libxgui.a").generic_u8string());
    for (const char *name : {"libxnetapi.a", "libxnet.a", "libxmedia.a"}) {
        const fs::path library = directory / name;
        if (fs::is_regular_file(library)) result.libraries.push_back(library.u8string());
    }
    result.libraries.push_back((directory / "liballoc.a").generic_u8string());
    return result;
}

std::vector<std::string> compile_command(const Options &options, const Profile &profile, const fs::path &sdk)
{
    std::string compiler = environment(profile.cpp ? "XXCC_CXX" : "XXCC_CC");
    if (compiler.empty()) compiler = profile.cpp ? "clang++" : "clang";
    std::vector<std::string> result = {compiler};
#ifdef _WIN32
    result.push_back("--target=x86_64-unknown-elf");
#endif
    // These are the original target flags: the host driver's C++ standard does
    // not change the application's ABI or its default C/C++ language standard.
    const std::vector<std::string> defaults = {
        "-Wall", "-ffreestanding", "-fno-builtin", "-m64", profile.cpp ? "-std=c++11" : "-std=c11",
        "-fno-stack-protector", "-fno-exceptions", "-fshort-wchar", "-nostdinc",
        "-I", (sdk / "include").u8string(), "-Wno-writable-strings", "-D__XJ380_OS__"
    };
    result.insert(result.end(), defaults.begin(), defaults.end());
    result.insert(result.end(), options.compile_args.begin(), options.compile_args.end());
    return result;
}

void validate_output(const fs::path &output, const Options &options)
{
    if (fs::is_directory(output)) throw std::runtime_error("output is a directory: '" + output.u8string() + "'");
    if (!fs::is_directory(fs::absolute(output).parent_path()))
        throw std::runtime_error("output directory does not exist: '" + output.parent_path().u8string() + "'");
    for (const Input &item : options.link_items) {
        if (!item.file) continue;
        const fs::path input = fs::u8path(item.value);
        if (fs::absolute(input).lexically_normal() == fs::absolute(output).lexically_normal() ||
            (fs::exists(output) && fs::equivalent(input, output)))
            throw std::runtime_error("output would overwrite input: '" + item.value + "'");
    }
}

fs::path stage_output(const Input &input, Stage stage)
{
    fs::path result = fs::u8path(input.value).filename();
    result.replace_extension(stage == Stage::Assembly ? ".s" : ".o");
    return result;
}

void add_dependencies(std::vector<std::string> &command, const Options &options, const fs::path &target)
{
    if (!options.dependencies) return;
    if (!options.dependency_file) {
        fs::path dependency = target;
        dependency.replace_extension(".d");
        command.insert(command.end(), {"-MF", dependency.u8string()});
    }
    if (!options.dependency_target) command.insert(command.end(), {"-MQ", target.u8string()});
}

void help()
{
    std::cout << "XINGJI XJ380 C/C++ Compiler 2026v5\n"
                 "Usage: xxcc [options] files... [-o output]\n"
                 "  -c / -S / -E          Compile / emit assembly / preprocess only\n"
                 "  --xxcc-sdk=PATH       SDK root containing include/ and lib/x86_64/\n"
                 "  --xxcc-save-temps     Keep intermediate objects for debugging\n"
                 "  -v                    Show commands (alone: show version)\n"
                 "  --version             Show version\n"
                 "SDK: --xxcc-sdk > XXCC_SDK_ROOT > executable-relative lookup\n"
                 "Tools: XXCC_CC, XXCC_CXX, XXCC_LD specify executable paths\n";
}

} // namespace

int run_driver(const std::vector<std::string> &args, const Profile &profile)
{
    if (args.size() <= 1 || (args.size() == 2 && (args[1] == "--help" || args[1] == "-h"))) {
        help();
        return 0;
    }
    if (args.size() == 2 && (args[1] == "--version" || args[1] == "-version" || args[1] == "-v")) {
        std::cout << "XINGJI XJ380 C/C++ Compiler 2026v5\n";
        return 0;
    }
    Options options = parse(args);
    std::size_t source_count = 0;
    std::size_t file_count = 0;
    for (const Input &input : options.link_items) {
        if (!input.file) continue;
        ++file_count;
        require_file(fs::u8path(input.value));
        if (is_source(input)) ++source_count;
    }
    if (!file_count) throw std::runtime_error("no input files");
    if (options.stage != Stage::Link && !source_count) throw std::runtime_error("no source files");
    if (options.stage != Stage::Link && source_count > 1 && !options.output.empty())
        throw std::runtime_error("cannot use one -o output with multiple source files in this mode");
    const fs::path final_output = fs::u8path(options.output.empty() ? (profile.elf ? "a.elf" : "a.epf") : options.output);
    if (options.stage == Stage::Link) validate_output(final_output, options);
    const fs::path sdk = find_sdk(options, args[0], profile);
    Runtime runtime;
    if (options.stage == Stage::Link) runtime = load_runtime(sdk, profile);
    const auto defaults = compile_command(options, profile, sdk);
    std::unique_ptr<TemporaryDirectory> work;
    if (options.stage == Stage::Link && source_count) {
        work.reset(new TemporaryDirectory(fs::temp_directory_path(), options.keep));
        if (options.keep) std::cerr << "xxcc: keeping temporary files in '" << work->path().u8string() << "'\n";
    }
    std::vector<std::string> user_link;
    std::size_t index = 0;
    for (const Input &input : options.link_items) {
        if (!is_source(input)) {
            user_link.push_back(input.file ? fs::absolute(fs::u8path(input.value)).u8string() : input.value);
            continue;
        }
        auto command = defaults;
        if (!input.language.empty()) command.insert(command.end(), {"-x", input.language});
        command.push_back(options.stage == Stage::Preprocess ? "-E" : options.stage == Stage::Assembly ? "-S" : "-c");
        fs::path output;
        std::unique_ptr<TemporaryDirectory> transaction;
        if (options.stage == Stage::Link) {
            output = work->path() / ("input-" + std::to_string(index++) + ".o");
            add_dependencies(command, options, source_count == 1 ? final_output : stage_output(input, Stage::Object));
        } else if (options.stage != Stage::Preprocess || (!options.output.empty() && options.output != "-")) {
            const fs::path destination = options.output.empty() ? stage_output(input, options.stage) : fs::u8path(options.output);
            validate_output(destination, options);
            transaction.reset(new TemporaryDirectory(fs::absolute(destination).parent_path()));
            output = transaction->path() / destination.filename();
            add_dependencies(command, options, destination);
        }
        if (!output.empty()) command.insert(command.end(), {"-o", output.u8string()});
        command.insert(command.end(), {"--", input.value});
        const int status = run_process(command, options.verbose);
        if (status) {
            std::cerr << "xxcc: compilation failed for '" << input.value << "' (exit " << status << ")\n";
            return status;
        }
        if (transaction) {
            const fs::path destination = options.output.empty() ? stage_output(input, options.stage) : fs::u8path(options.output);
            publish_file(output, destination);
        }
        if (options.stage == Stage::Link) {
            require_file(output);
            user_link.push_back(output.u8string());
        }
    }
    if (options.stage != Stage::Link) return 0;
    TemporaryDirectory transaction(fs::absolute(final_output).parent_path());
    const fs::path staged = transaction.path() / final_output.filename();
    std::string linker = environment("XXCC_LD");
#ifdef _WIN32
    if (linker.empty()) linker = "lld";
    std::vector<std::string> command = {linker, "-flavor", "gnu", "-m", "elf_x86_64", "-Ttext=0x200000"};
#else
    if (linker.empty()) linker = "ld";
    std::vector<std::string> command = {linker, "-Ttext=0x200000"};
#endif
    command.insert(command.end(), runtime.objects.begin(), runtime.objects.end());
    command.push_back(runtime.startup.u8string());
    command.insert(command.end(), user_link.begin(), user_link.end());
    command.insert(command.end(), runtime.libraries.begin(), runtime.libraries.end());
    command.insert(command.end(), {"-o", staged.u8string()});
    const int status = run_process(command, options.verbose);
    if (status) {
        std::cerr << "xxcc: linking failed (exit " << status << ")\n";
        return status;
    }
    publish_file(staged, final_output);
    return 0;
}

} // namespace xxcc
