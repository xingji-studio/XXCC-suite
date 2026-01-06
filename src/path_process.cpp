#include <iostream>
#include <proto.h>

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
