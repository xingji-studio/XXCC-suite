#include "include/driver.h"
#include "include/platform.h"

#include <exception>
#include <iostream>

#if defined(XXCC_HOST_WINDOWS) && !defined(_WIN32)
#error Windows drivers require a Windows-targeting C++ compiler (for example MinGW).
#endif
#if defined(XXCC_HOST_LINUX) && defined(_WIN32)
#error Linux drivers require a Linux-targeting C++ compiler.
#endif

int main(int argc, char **argv)
{
    const xxcc::Profile profile = {
#ifdef XXCC_CPP
        true,
#else
        false,
#endif
#ifdef XXCC_TUI
        true,
#else
        false,
#endif
#ifdef XXCC_ELF
        true
#else
        false
#endif
    };

    try {
        return xxcc::run_driver(xxcc::arguments(argc, argv), profile);
    } catch (const std::exception &error) {
        std::cerr << "xxcc: error: " << error.what() << '\n';
        return 1;
    }
}
