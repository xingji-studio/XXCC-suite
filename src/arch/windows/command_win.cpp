#include <iostream>
#include <proto.h>
#include <string.h>

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
