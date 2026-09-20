#include <network.h>
#include <ethernet/ethernet.h>
#include <e1000.h>
#include <printf.h>

#define NETWORK_MAX_DEVICES 4

static network_device_t *network_devices[NETWORK_MAX_DEVICES];
static int network_device_count = 0;
static network_device_t *primary_network_device = (void *)0;

int network_register_device(network_device_t *device)
{
  if (!device || !device->ops || !device->ops->send || !device->ops->poll) {
    return -1;
  }

  if (network_device_count >= NETWORK_MAX_DEVICES) {
    return -1;
  }

  network_devices[network_device_count++] = device;
  if (!primary_network_device) {
    primary_network_device = device;
  }

  return 0;
}

network_device_t *network_primary_device(void)
{
  return primary_network_device;
}

const uint8_t *network_primary_mac(void)
{
  if (!primary_network_device) {
    return (void *)0;
  }

  return primary_network_device->mac;
}

const uint8_t *network_primary_ipv4(void)
{
  if (!primary_network_device) {
    return (void *)0;
  }

  return primary_network_device->ipv4;
}

int network_send(const void *data, uint16_t length)
{
  if (!primary_network_device) {
    return -1;
  }

  return primary_network_device->ops->send(primary_network_device, data, length);
}

int network_poll(void)
{
  int processed = 0;

  for (int index = 0; index < network_device_count; index++) {
    processed += network_devices[index]->ops->poll(network_devices[index]);
  }

  return processed;
}

void network_handle_frame(void *frame, uint16_t length)
{
  handle_ethernet_frame((ethernet_frame_header_t *)frame, length);
}

void network_rx_worker(void)
{
  for (;;) {
    if (network_poll() == 0) {
      __asm__ volatile ("hlt");
    }
  }
}

int network_init(void)
{
  static network_device_t e1000_device = {
    .name = "e1000",
    .ipv4 = {10, 0, 2, 2},
  };

  if (e1000_init(&e1000_device) != 0) {
    kprintf("network: no supported NIC initialized\n");
    return -1;
  }

  if (network_register_device(&e1000_device) != 0) {
    kprintf("network: failed to register device %s\n", e1000_device.name);
    return -1;
  }

  kprintf("network: registered device %s\n", e1000_device.name);
  return 0;
}