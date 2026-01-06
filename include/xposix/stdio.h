#pragma once

#include "../stdint.h"

int read(int fd, char *buf, uint64_t len);
int write(int fd, char *buf, uint64_t len);
