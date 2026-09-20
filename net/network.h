#pragma once

#include <stdint.h>

typedef struct network_device network_device_t;

typedef int (*network_send_fn)(network_device_t *device, const void *data, uint16_t length);
typedef int (*network_poll_fn)(network_device_t *device);

typedef struct network_device_ops {
  network_send_fn send;
  network_poll_fn poll;
} network_device_ops_t;

struct network_device {
  const char *name;
  uint8_t mac[6];
  uint8_t ipv4[4];
  void *driver_data;
  const network_device_ops_t *ops;
};

int network_init(void);
int network_register_device(network_device_t *device);
network_device_t *network_primary_device(void);
const uint8_t *network_primary_mac(void);
const uint8_t *network_primary_ipv4(void);
int network_send(const void *data, uint16_t length);
int network_poll(void);
void network_handle_frame(void *frame, uint16_t length);
void network_rx_worker(void);