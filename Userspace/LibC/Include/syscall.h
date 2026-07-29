#pragma once

#include <stdint.h>

uint64_t syscall(uint64_t number, uint64_t a, uint64_t b, uint64_t c);

void exit(int status);
uint64_t write(int fd, const void* buffer, uint64_t count);
