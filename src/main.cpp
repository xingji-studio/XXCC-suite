#include <iostream>
#include <stdlib.h>
#include <string.h>
#include "./include/proto.h"

#ifdef XXCC_LINUX
#include <dirent.h>
#include <sys/stat.h>
#endif

#ifdef XXCC_WINDOWS
#include <io.h>
#endif

using namespace std;

char   *compile_arg[256];
char   *input_file_name[256];
char   *object_file_name[256];
char    output_file_name[256];
int     input_file_count = 0;
int     input_arg_count = 0;
bool    use_Clang = false;
bool    use_Gcc   = false;
const int MAX_AUTO_OBJECT_FILE_COUNT = 512;
char auto_object_file_list[MAX_AUTO_OBJECT_FILE_COUNT][512];
int auto_object_file_count = 0;

bool has_suffix(const char *text, const char *suffix)
{
    size_t text_length = strlen(text);
    size_t suffix_length = strlen(suffix);

    if (text_length < suffix_length) return false;

    return strcmp(text + text_length - suffix_length, suffix) == 0;
}

void normalize_path_separator(char *path)
{
    while (*path)
    {
        if (*path == '\\') *path = '/';
        path++;
    }
}

bool should_skip_object_file(const char *path)
{
    return strcmp(path, "start.cpp.o") == 0 ||
           strcmp(path, "constart.cpp.o") == 0 ||
           has_suffix(path, "/start.cpp.o") ||
           has_suffix(path, "/constart.cpp.o") ||
           has_suffix(path, "\\start.cpp.o") ||
           has_suffix(path, "\\constart.cpp.o");
}

void add_object_file(const char *path)
{
    if (auto_object_file_count >= MAX_AUTO_OBJECT_FILE_COUNT) return;

    strcpy(auto_object_file_list[auto_object_file_count], path);
    normalize_path_separator(auto_object_file_list[auto_object_file_count]);
    auto_object_file_count++;
}

int compare_object_file_name(const void *left, const void *right)
{
    return strcmp((const char *)left, (const char *)right);
}

#ifdef XXCC_LINUX

void collect_object_files_recursive(const char *dir_path)
{
    DIR *dir = opendir(dir_path);
    if (dir == NULL) return;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

        char file_path[1024];
        memset(file_path, 0, sizeof(file_path));
        strcat(file_path, dir_path);
        strcat(file_path, "/");
        strcat(file_path, entry->d_name);

        struct stat entry_stat;
        if (stat(file_path, &entry_stat) != 0) continue;

        if (S_ISDIR(entry_stat.st_mode))
        {
            collect_object_files_recursive(file_path);
            continue;
        }

        if (S_ISREG(entry_stat.st_mode) && has_suffix(file_path, ".o") && !should_skip_object_file(file_path))
        {
            add_object_file(file_path);
        }
    }

    closedir(dir);
}

#endif

#ifdef XXCC_WINDOWS

void collect_object_files_recursive(const char *dir_path)
{
    char search_path[1024];
    memset(search_path, 0, sizeof(search_path));
    strcat(search_path, dir_path);
    strcat(search_path, "\\*");

    struct _finddata_t entry;
    intptr_t handle = _findfirst(search_path, &entry);
    if (handle == -1) return;

    do
    {
        if (strcmp(entry.name, ".") == 0 || strcmp(entry.name, "..") == 0) continue;

        char file_path[1024];
        memset(file_path, 0, sizeof(file_path));
        strcat(file_path, dir_path);
        strcat(file_path, "\\");
        strcat(file_path, entry.name);

        if (entry.attrib & _A_SUBDIR)
        {
            collect_object_files_recursive(file_path);
            continue;
        }

        if (has_suffix(file_path, ".o") && !should_skip_object_file(file_path))
        {
            add_object_file(file_path);
        }
    }
    while (_findnext(handle, &entry) == 0);

    _findclose(handle);
}

#endif

int main(int argc, char *argv[], char *envp[])
{
    if (argc <= 1)
    {
        // 没有给任何参数
        cout << "XINGJI XJ380 C/C++ Compiler" << endl;
        cout << "Copyright(C) XINGJI Interactive Software 2017 - 2026 All rights reserved." << endl;
        cout << "A XINGJI XJ380 Develop Department Production. XINGJI Present." << endl;
        cout << "Visit https://www.xingjisoft.com/os/download and download XJ380 API Specification Document to learn how to compile a file to XJ380 application." << endl;
        cout << "Please install Clang 18.1.8 or higher version compiler first when you use XXCC." << endl;
        return 0;
    }

    if (argc == 2)
    {
        // 有一个参数
        if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "-version") == 0)
        {
            cout << "XINGJI XJ380 C/C++ Compiler 2026v4" << endl;
            cout << "Copyright(C) XINGJI Interactive Software 2017 - 2026 All rights reserved." << endl;
            cout << "A XINGJI XJ380 Develop Department Production. XINGJI Present." << endl;
            return 0;
        }
    }

    int i;
    for (i = 1; i < argc; i++)
    {
        // 复制编译参数
        if (argv[i][0] != '-') break;
        compile_arg[i - 1] = (char *)malloc(strlen(argv[i]) + 1);
        strcpy(compile_arg[i - 1], argv[i]);
        input_arg_count++;
    }

    for (; i < argc; i++)
    {
        // 复制文件名
        if (argv[i][0] == '-') break;
        input_file_name[i - 1] = (char *)malloc(strlen(argv[i]) + 1);
        strcpy(input_file_name[i - 1], argv[i]);
        input_file_count++;
    }

    i++; // 跳过 -o

    strcpy(output_file_name, argv[i]);

    // 准备输出文件夹
    create_dir("xxcc-temp-out");

    // 编译
    // -Wall -ffreestanding -fno-builtin -m64 -std=c++11 -fno-stack-protector -fno-exceptions -fshort-wchar -nostdinc -Wno-writable-strings -c $< -o $@
    for (i = 0; i < input_file_count; i++)
    {
        char command[512];
        char input_name[256];
        char output_name[256];
        memset(command, 0, 512);
        memset(input_name, 0, 256);
        memset(output_name, 0, 256);

        // 获取文件名，合成输出文件名（XXX/XXX.cpp -> ./xxcc-temp-out/XXX.cpp.o）
        get_file_name(input_name, input_file_name[i]);
        strcat(output_name, "./xxcc-temp-out/");
        strcat(output_name, input_name);
        strcat(output_name, ".o");

        // 保存输出文件名
        object_file_name[i] = (char *)malloc(strlen(output_name) + 1);
        strcpy(object_file_name[i], output_name);

        // 合成编译命令
#ifdef XXCC_CPP

    #ifdef XXCC_WINDOWS
        strcat(command, "clang++ --target=x86_64-unknown-elf ");
    #else
        strcat(command, "clang++ ");
    #endif

        strcat(command, "-Wall -ffreestanding -fno-builtin -m64 -std=c++11 -fno-stack-protector -fno-exceptions -fshort-wchar -nostdinc -I ./include -Wno-writable-strings -D __XJ380_OS__ -c ");
#else

    #ifdef XXCC_WINDOWS
        strcat(command, "clang --target=x86_64-unknown-elf ");
    #else
        strcat(command, "clang ");
    #endif

        strcat(command, "-Wall -ffreestanding -fno-builtin -m64 -std=c11 -fno-stack-protector -fno-exceptions -fshort-wchar -nostdinc -I ./include -Wno-writable-strings -D __XJ380_OS__ -c ");
#endif

        for (int j = 0; j < input_arg_count; j++)
        {
            strcat(command, compile_arg[j]);
            strcat(command, " ");
        }

        strcat(command, input_file_name[i]);
        strcat(command, " -o ");
        strcat(command, output_name);

        // 编译
        system(command);
    }

    // 生成链接命令
    char linker_command[65536];
    memset(linker_command, 0, sizeof(linker_command));
#ifdef XXCC_WINDOWS
    strcat(linker_command, "lld -flavor gnu -m elf_x86_64 -Ttext=0x200000 ");
#else
    strcat(linker_command, "ld -Ttext=0x200000 ");
#endif

    auto_object_file_count = 0;

    // 自动递归收集运行时目标文件，启动对象单独拼接。
#ifdef XXCC_GUI
    collect_object_files_recursive("./obj-gui");
    qsort(auto_object_file_list, auto_object_file_count, sizeof(auto_object_file_list[0]), compare_object_file_name);
    for (int i = 0; i < auto_object_file_count; i++)
    {
        strcat(linker_command, auto_object_file_list[i]);
        strcat(linker_command, " ");
    }
    strcat(linker_command, "./obj-gui/liballoc-x86_64.a ");
#elif defined(XXCC_TUI)
    collect_object_files_recursive("./obj-tui");
    qsort(auto_object_file_list, auto_object_file_count, sizeof(auto_object_file_list[0]), compare_object_file_name);
    for (int i = 0; i < auto_object_file_count; i++)
    {
        strcat(linker_command, auto_object_file_list[i]);
        strcat(linker_command, " ");
    }
    strcat(linker_command, "./obj-tui/liballoc-x86_64.a ");
#endif

    // 根据情况选择
#ifdef XXCC_GUI
    strcat(linker_command, "./obj-gui/start.cpp.o ");
#elif defined(XXCC_TUI)
    strcat(linker_command, "./obj-tui/constart.cpp.o ");
#endif

    // 拼接目标文件
    for (i = 0; i < input_file_count; i++)
    {
        strcat(linker_command, object_file_name[i]);
        strcat(linker_command, " ");
    }

    strcat(linker_command, "-o ");
    strcat(linker_command, output_file_name);

    // 链接
    system(linker_command);

    // 完成任务！
    delete_dir("xxcc-temp-out");

    return 0;
}

#ifdef XXCC_LINUX

void create_dir(char *path)
{
    char command[256];
    memset(command, 0, 256);
    strcat(command, "mkdir ");
    strcat(command, path);
    system(command);
}

#endif

#ifdef XXCC_WINDOWS

void create_dir(char *path)
{
    char command[256];
    memset(command, 0, 256);
    strcat(command, "mkdir ");
    strcat(command, path);
    system(command);
}

#endif

#ifdef XXCC_LINUX

void delete_dir(char *path)
{
    char command[256];
    memset(command, 0, 256);
    strcat(command, "rmdir ");
    strcat(command, path);
    system(command);
}

#endif

#ifdef XXCC_WINDOWS

void delete_dir(char *path)
{
    char command[256];
    memset(command, 0, 256);
    strcat(command, "rmdir ");
    strcat(command, path);
    strcat(command, " /s /q");
    system(command);
}

#endif

void get_file_name(char *name, char *path)
{
    char *p = path;
    char *q = name;

    while (*p)
        p++;

    while (p != path && *p != '\\' && *p != '/')
        p--;

    if (p != path) p++;

    while (*p)
    {
        *q = *p;
        p++; q++;
    }
}
