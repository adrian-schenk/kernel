#pragma once
#include <stddef.h>

void strcpy(char*, char*);
void strncpy(char*, char*, unsigned int len);

int memcmp4(const char *a, const char *b);
int memcmp8(const char*, const char*);

int strlen(char*);
int str2int(char*);
int is_int(char c);

int strcmp(const char *str1, const char *str2);
int strncmp(const char *s1, const char *s2, size_t n);