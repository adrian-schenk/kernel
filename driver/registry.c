#include <registry.h>

driver_registry_t *driver_registry[] = {
    &driver_e1000_registry,
    0 /* sentinel */
};