#pragma once

typedef struct {
    char* name;
    int (*init)(void*);
    void (*shutdown)(void);
    char (*can_activate)(void*);
} driver_registry_t;

#define DECLARE_DRIVER(name) extern const driver_registry_t driver_##name##_registry;
#define REGISTER_DRIVER(name) const driver_registry_t driver_##name##_registry