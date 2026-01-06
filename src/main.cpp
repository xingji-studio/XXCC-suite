#include <iostream>
#include <string.h>
#include <proto.h>

using namespace std;

char   *compile_arg[256];
char   *input_file_name[256];
char   *object_file_name[256];
char    output_file_name[256];
int     input_file_count = 0;
int     input_arg_count = 0;
bool    use_Clang = false;
bool    use_Gcc   = false;

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
        strcat(command, "clang++ ");
        strcat(command, "-Wall -ffreestanding -fno-builtin -m64 -std=c++11 -fno-stack-protector -fno-exceptions -fshort-wchar -nostdinc -I ./include -Wno-writable-strings -c ");

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
    char linker_command[1024];
    memset(linker_command, 0, 1024);
    strcat(linker_command, "ld -z muldefs -Ttext=0x200000 ");
    strcat(linker_command, "./obj-gui/xtuiapi.cpp.o ");
    strcat(linker_command, "./obj-gui/xguiapi.cpp.o ");
    strcat(linker_command, "./obj-gui/unistd.cpp.o ");
    strcat(linker_command, "./obj-gui/stdlib.cpp.o ");
    strcat(linker_command, "./obj-gui/stdio.cpp.o ");
    strcat(linker_command, "./obj-gui/libsys.cpp.o ");
    strcat(linker_command, "./obj-gui/krlibc.cpp.o ");
    strcat(linker_command, "./obj-gui/arch/x86_64/crt0.S.o ");

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

    return 0;
}
