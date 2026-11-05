#pragma once

#include <string>
#include <vector>

namespace xxcc {

struct Profile {
    bool cpp;
    bool tui;
    bool elf;
};

int run_driver(const std::vector<std::string> &args, const Profile &profile);

} // namespace xxcc
