#pragma once

#include <stdint.h>

namespace Drivers::VirtIO {

constexpr uint8_t VIRTIO_PCI_CAP_COMMON_CFG = 1;
constexpr uint8_t VIRTIO_PCI_CAP_NOTIFY_CFG = 2;
constexpr uint8_t VIRTIO_PCI_CAP_ISR_CFG = 3;
constexpr uint8_t VIRTIO_PCI_CAP_DEVICE_CFG = 4;
constexpr uint8_t VIRTIO_PCI_CAP_PCI_CFG = 5;

struct ConfigurationRegion {
	uint8_t bar = 0xFF;
	uint32_t offset;
	uint32_t length;

	volatile uint8_t* bar_base;
	volatile uint8_t* address;
};

struct ConfigurationPointers {
	ConfigurationRegion common;
	ConfigurationRegion notify;
	ConfigurationRegion isr;
	ConfigurationRegion device;
};

// 2.1 Device Status Field
enum class DeviceStatusFlags : uint8_t {
	Acknowledge = 1,
	Driver = 2,
	Failed = 128,
	FeaturesOk = 8,
	DriverOk = 4,
	DeviceNeedsReset = 64
};

// 4.1.4.3 Common configuration structure layout
struct alignas(4) CommonConfig {
	/* About the whole device. */

	uint32_t device_feature_select;
	uint32_t device_feature;
	uint32_t driver_feature_select;
	uint32_t driver_feature;
	uint16_t msix_config;
	uint16_t num_queues;
	uint8_t device_status;
	uint8_t config_generation;

	/* About a specific virtqueue. */

	uint16_t queue_select;
	uint16_t queue_size;
	uint16_t queue_msix_vector;
	uint16_t queue_enable;
	uint16_t queue_notify_off;
	uint64_t queue_desc;
	uint64_t queue_driver;
	uint64_t queue_device;
} __attribute__((packed));

}
