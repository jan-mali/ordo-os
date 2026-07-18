#include <Arch/x86_64/GDT.hpp>
#include <Arch/x86_64/IO.hpp>
#include <Arch/x86_64/Interrupts.hpp>
#include <Arch/x86_64/Serial.hpp>
#include <Core/ACPI/ACPI.hpp>
#include <Core/ACPI/Timer.hpp>
#include <Core/MM/KernelHeap.hpp>
#include <Core/MM/KernelVMM.hpp>
#include <Core/MM/PMM.hpp>
#include <Core/MM/Paging.hpp>
#include <Core/Multitasking/FinalizerTask.hpp>
#include <Core/Multitasking/Scheduler.hpp>
#include <Core/Multitasking/Thread.hpp>
#include <Core/PCI/PCI.hpp>
#include <Core/Panic.hpp>
#include <Core/TextConsole.hpp>
#include <Logger/Logger.hpp>
#include <Logger/kstdio.hpp>
#include <cstddef>
#include <cstdint>
#include <limine.h>

// Set the base revision to 6, this is recommended as this is the latest
// base revision described by the Limine boot protocol specification.
// See specification for further info.

namespace {

__attribute__((used, section(".limine_requests"))) volatile std::uint64_t limine_base_revision[] = LIMINE_BASE_REVISION(6);

__attribute__((used, section(".limine_requests"))) volatile limine_framebuffer_request framebuffer_request = {
	.id = LIMINE_FRAMEBUFFER_REQUEST_ID,
	.revision = 0,
	.response = nullptr
};

__attribute__((used, section(".limine_requests"))) volatile limine_bootloader_info_request bootloader_info_request = {
	.id = LIMINE_BOOTLOADER_INFO_REQUEST_ID,
	.revision = 0,
	.response = nullptr
};

__attribute__((used, section(".limine_requests"))) volatile limine_hhdm_request hhdm_request = {
	.id = LIMINE_HHDM_REQUEST_ID,
	.revision = 0,
	.response = nullptr
};

__attribute__((used, section(".limine_requests"))) volatile limine_memmap_request memmap_request = {
	.id = LIMINE_MEMMAP_REQUEST_ID,
	.revision = 0,
	.response = nullptr
};

__attribute__((used, section(".limine_requests"))) volatile limine_rsdp_request rsdp_request = {
	.id = LIMINE_RSDP_REQUEST_ID,
	.revision = 0,
	.response = nullptr
};

__attribute__((used, section(".limine_requests_start"))) volatile std::uint64_t limine_requests_start_marker[] = LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end"))) volatile std::uint64_t limine_requests_end_marker[] = LIMINE_REQUESTS_END_MARKER;

}

[[noreturn]] void halt() {
	while (true) {
		asm volatile("hlt");
	}
}

// The following stubs are required by the Itanium C++ ABI (the one we use,
// regardless of the "Itanium" nomenclature).
// Like the memory functions above, these stubs can be moved to a different .cpp file,
// but should not be removed, unless you know what you are doing.
extern "C" {
int __cxa_atexit(void (*)(void*), void*, void*) { return 0; }
void __cxa_pure_virtual() { halt(); }
void* __dso_handle;
}

using constructor = void (*)();

extern "C" constructor __init_array[];
extern "C" constructor __init_array_end[];

void init_stage2(void*);

extern "C" void kmain() {
	// This is guaranteed by Limine but we ensure it here for sanity reasons
	Interrupts::disable();

	// Call global constructors.
	for (constructor* ctor = __init_array; ctor != __init_array_end; ctor++) {
		(*ctor)();
	}

	// Get the serial working as soon as possible to have any debugging capabilities
	serial.init();
	serial.write("Hello serial!\n");

	// Ensure the bootloader actually understands our base revision (see spec).
	if (LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision) == false) {
		panic("Bootloader doesn't support the requested Limine protocol revision.\n");
	}

	// Ensure we got a framebuffer.
	if (framebuffer_request.response == nullptr
		|| framebuffer_request.response->framebuffer_count < 1) {
		panic("No framebuffer received.\n");
	}

	// Fetch the first framebuffer.
	limine_framebuffer* framebuffer = framebuffer_request.response->framebuffers[0];

	// Print a nice pattern to screen as an example.
	// Note: we assume the framebuffer model is RGB with 32-bit pixels.
	volatile std::uint32_t* fb_ptr = static_cast<volatile std::uint32_t*>(framebuffer->address);
	for (std::size_t y = 0; y < framebuffer->height; y++) {
		for (std::size_t x = 0; x < framebuffer->width; x++) {
			std::uint32_t nX = x * 255 / framebuffer->width;
			std::uint32_t nY = y * 255 / framebuffer->height;
			fb_ptr[y * (framebuffer->pitch / 4) + x] = (nY << 8) | nX;
		}
	}

	text_console.init(framebuffer->pitch, framebuffer->height, fb_ptr);

	Logger::kprintf("\n");
	Logger::kprintf("   ___          _          ___  ____  \n");
	Logger::kprintf("  / _ \\ _ __ __| | ___    / _ \\/ ___| \n");
	Logger::kprintf(" | | | | '__/ _` |/ _ \\  | | | \\___ \\ \n");
	Logger::kprintf(" | |_| | | | (_| | (_) | | |_| |___) |\n");
	Logger::kprintf("  \\___/|_|  \\__,_|\\___/   \\___/|____/ \n");
	Logger::kprintf("\n\n");

	Logger::log_info("Booting Ordo OS v0.0.1-dev\n");

	if (auto* response = bootloader_info_request.response) {
		Logger::log_info("Bootloader information: %s, version %s.\n", response->name, response->version);
	}

	GDT::init();

	Interrupts::init();

	pmm.init(memmap_request.response, hhdm_request.response);

	Paging::init();

	kernel_vmm.init();

	kernel_heap.init();

	ACPI::init(rsdp_request.response);
	ACPI::Timer::init();

	Multitasking::init();
	Multitasking::FinalizerTask::start();

	Multitasking::scheduler->add_thread(Multitasking::create_thread(init_stage2, nullptr, "init_stage2"));

	// The Scheduler also enables interrupts
	Multitasking::scheduler->start();

	panic("Reached end of kmain\n");
}

void init_stage2(void*) {
	Logger::log_info("Continuing boot from init_stage2! :)\n");

	PCI::enumerate();
}
