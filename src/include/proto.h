#pragma once

#if defined(_WIN32) || defined(_WIN64)
#define XXCC_WINDOWS
#else
#define XXCC_LINUX
#endif
#define XXCC_GUI
#define XXCC_EPF
#define XXCC_CPP

// command.cpp
void create_dir(char *path);
void delete_dir(char *path);

// path_process.cpp
void get_file_name(char *name, char *path);
