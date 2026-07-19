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

// 5.2.4 Device configuration layout
// https://docs.oasis-open.org/virtio/virtio/v1.1/cs01/virtio-v1.1-cs01.html#x1-2440004
struct BlockDeviceConfig {
	uint64_t capacity;
	uint32_t size_max;
	uint32_t seg_max;
	struct virtio_blk_geometry {
		uint16_t cylinders;
		uint8_t heads;
		uint8_t sectors;
	} __attribute__((packed)) geometry;
	uint32_t blk_size;
	struct virtio_blk_topology {
		// # of logical blocks per physical block (log2)
		uint8_t physical_block_exp;
		// offset of first aligned logical block
		uint8_t alignment_offset;
		// suggested minimum I/O size in blocks
		uint16_t min_io_size;
		// optimal (suggested maximum) I/O size in blocks
		uint32_t opt_io_size;
	} __attribute__((packed)) topology;
	uint8_t writeback;
	uint8_t unused0[3];
	uint32_t max_discard_sectors;
	uint32_t max_discard_seg;
	uint32_t discard_sector_alignment;
	uint32_t max_write_zeroes_sectors;
	uint32_t max_write_zeroes_seg;
	uint8_t write_zeroes_may_unmap;
	uint8_t unused1[3];
} __attribute__((packed));

}
