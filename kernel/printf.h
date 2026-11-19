#pragma once
#include <stdarg.h>
#include "string.h"
#include "console.h"

#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define NC "\033[0m"

int kprintf(const char* str, ...);
static unsigned long long kprintf_parse_color(char **str);