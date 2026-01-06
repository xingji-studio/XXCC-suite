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

void change_exname(char *path, char *exname)
{
    char *p = path;
    while (*p)
        p++;

    while (*p != '.')
        p--;

    p++;

    char *q = exname;
    while (*q)
    {
        *p = *q;
        p++; q++;
    }
}
