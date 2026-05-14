#pragma once

#if defined(_WIN32) || defined(_WIN64)
#define XXCC_WINDOWS
#else
#define XXCC_LINUX
#endif

// command.cpp
void create_dir(const char *path);
void delete_dir(const char *path);

// path_process.cpp
void get_file_name(char *name, char *path);
