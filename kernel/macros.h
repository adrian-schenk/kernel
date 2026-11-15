#pragma once

#define FOREACH(arr, val) \
    for (int i = 0; (arr)[i] != (void *)0; i++) \
        for (typeof(*(arr)) val = (arr)[i]; val != (void *)0; val = (void *)0)
#define ENDFOREACH()