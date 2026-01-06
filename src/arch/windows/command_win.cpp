#include <proto.h>

#ifdef XXCC_WINDOWS

void create_dir(char *path)
{
    char command[256];
    memset(command, 0, 256);
    strcat(command, "mkdir ");
    strcat(command, path);
    system(command);
}

void rename_file(char *new_name, char *path)
{
    char command[256];
    memset(command, 0, 256);
    strcat(command, "rename ");
    strcat(command, new_name);
    strcat(command, " ");
    strcat(command, path);
    system(command);
}

#endif
