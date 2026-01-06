#pragma once

#include "../stdint.h"

uint64_t fork();
uint64_t vfork();

uint64_t execve(char *filename, char *argv[], char *envp[]);
