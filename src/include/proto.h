#pragma once

#define XXCC_LINUX
#define XXCC_GUI
#define XXCC_EPF

// command.cpp
void create_dir(char *path);
void rename_file(char *new_name, char *path);

// path_process.cpp
void get_file_name(char *name, char *path);
void change_exname(char *path, char *exname);
