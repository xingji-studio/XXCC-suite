#include <iostream>
#include <proto.h>

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
