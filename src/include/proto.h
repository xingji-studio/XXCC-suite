#pragma once

#if defined(_WIN32) || defined(_WIN64)
#define XXCC_WINDOWS
#else
#define XXCC_LINUX
#endif
#define XXCC_GUI
#define XXCC_EPF

// command.cpp
void create_dir(char *path);

// path_process.cpp
void get_file_name(char *name, char *path);
void change_exname(char *path, char *exname);
