#pragma once

extern int rand_seed;

static inline int krand(void) {
    rand_seed ^= rand_seed << 13;
    rand_seed ^= rand_seed >> 17;
    rand_seed ^= rand_seed << 5;

    return rand_seed;
}