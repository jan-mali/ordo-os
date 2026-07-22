#pragma once

#include <Arch/x86_64/Registers.hpp>
#include <Core/Multitasking/Thread.hpp>
#include <Core/PCI/Device.hpp>
#include <Drivers/BlockDevice.hpp>
#include <Drivers/VirtIO/VirtIOCapability.hpp>
#include <Drivers/VirtIO/VirtQueue.hpp>
#include <Utility/IRQSpinlock.hpp>
#include <Utility/IRQWaitQueue.hpp>
#include <cstdint>

namespace Drivers::VirtIO {

// Standard VirtIO 1.0+ Block Request Layout
struct virtio_blk_req {
	uint32_t type;
	uint32_t reserved;
	uint64_t sector;
};

constexpr int VIRTIO_BLK_T_IN = 0;
constexpr int VIRTIO_BLK_T_OUT = 1;

constexpr int VIRTQ_DESC_F_NEXT = 1;
constexpr int VIRTQ_DESC_F_WRITE = 2;

// Tracking structure allocated on the thread's stack
struct BlockRequest {
	RawSpinlock lock;
	IRQWaitQueue wq;
	volatile bool completed = false;
	volatile uint8_t status = 0xFF;

	virtio_blk_req header;
};

class VirtIOBlockDevice : public BlockDevice {
public:
	VirtIOBlockDevice(ConfigurationPointers config, PCI::Device* pci_device, uint32_t notify_offset_multiplier)
		: config(config)
		, pci_device(pci_device)
		, notify_offset_multiplier(notify_offset_multiplier) {}

	virtual ~VirtIOBlockDevice() = default;

	bool read(uint64_t lba, uint64_t count, uint8_t* buffer) override;
	bool write(uint64_t lba, uint64_t count, uint8_t* buffer) override;
	uint64_t sector_size() override { return 512; }
	uint64_t sector_count() override;

	bool initialize();

	void handle_interrupt(Registers* regs);

private:
	bool execute_request(uint32_t type, uint64_t lba, uint64_t count, volatile uint8_t* buffer);

	ConfigurationPointers config;
	PCI::Device* pci_device;
	uint32_t notify_offset_multiplier;

	VirtQueue* virtqueue;

	IRQSpinlock device_lock;			 // Protects VirtQueue rings & allocation
	BlockRequest** outstanding_requests; // Size matched to virtqueue->size
};

}
