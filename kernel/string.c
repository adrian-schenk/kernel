#include "string.h"

int strlen(char * str){
    int len = 0;
    while(*str){
        str++;
        len++;
    }
    return len;
}

void strncpy(char * dest, char * src, unsigned int n){
    for(int i = 0; i < n; i++){
        dest[i] = src[i];
    }
    dest[n-1] = '\0';
}

int is_int(char c){
    if(c >= '0' && c <= '9'){
        return 1;
    }
    return 0;
}

int memcmp4(const char *a, const char *b){
    for(int i = 0; i < 4; i++){
        if(a[i] != b[i]){
            return 0;
        }
    }
    return 1;
}

int memcmp8(const char *a, const char *b){
    for(int i = 0; i < 8; i++){
        if(a[i] != b[i]){
            return 0;
        }
    }
    return 1;
}