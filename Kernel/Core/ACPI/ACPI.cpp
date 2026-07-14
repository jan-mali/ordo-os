#include <Core/ACPI/ACPI.hpp>
#include <Core/ACPI/Interrupts.hpp>
#include <Core/ACPI/MADT.hpp>
#include <Core/MM/PMM.hpp>
#include <Core/Panic.hpp>
#include <Core/memory.hpp>
#include <Logger/Logger.hpp>
#include <Logger/kstdio.hpp>
#include <cstdint>

namespace ACPI {

static void setup_acpi_from_xsdt(XSDT* xsdt);
static void setup_acpi_from_rsdt(RSDT* rsdt);
static bool parse_root_entry(uint64_t entry);

XSDP* xsdp;

bool XSDP::checksum_correct() {
	uint8_t sum = 0;

	uint8_t* c = reinterpret_cast<uint8_t*>(this);
	for (uint32_t i = 0; i < 20; i++) {
		sum += *c++;
	}

	if (revision >= 2) {
		for (uint32_t i = 20; i < length; i++) {
			sum += *c++;
		}
	}

	return sum == 0;
}

bool SDTHeader::checksum_correct() {
	uint8_t sum = 0;

	uint8_t* c = reinterpret_cast<uint8_t*>(this);
	for (uint32_t i = 0; i < length; i++) {
		sum += *c++;
	}

	return sum == 0;
}

uint32_t* RSDT::entries() {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Waddress-of-packed-member"
	return &first_entry;
#pragma GCC diagnostic pop
}

uint32_t RSDT::entry_count() {
	return (header.length - sizeof(header)) / sizeof(uint32_t);
}

uint64_t* XSDT::entries() {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Waddress-of-packed-member"
	return &first_entry;
#pragma GCC diagnostic pop
}

uint64_t XSDT::entry_count() {
	return (header.length - sizeof(header)) / sizeof(uint64_t);
}

void init(limine_rsdp_response* rsdp_response) {
	if (!rsdp_response) {
		panic("No RSDP was provided.\n");
	}

	xsdp = reinterpret_cast<XSDP*>(rsdp_response->address);

	if (!xsdp->checksum_correct()) {
		panic("The RSDP checksum is incorrect.\n");
	}

	Logger::log_info("ACPI: RSDP with revision %d was found at address %X.\n", xsdp->revision, &xsdp);

	bool failure = false;
	if (xsdp->revision == 2) {
		XSDT* xsdt = reinterpret_cast<XSDT*>(pmm.translate_to_hhdm(xsdp->xsdt_address));
		if (xsdt->header.checksum_correct()) {
			Logger::log_info("ACPI: Setting up ACPI from the XSDT.\n");
			setup_acpi_from_xsdt(xsdt);
			return;
		} else {
			Logger::log_error("ACPI: XSDT is invalid, will try to set up ACPI from the RSDT.\n");
			failure = true;
		}
	}

	if (xsdp->revision == 1 || failure) {
		RSDT* rsdt = reinterpret_cast<RSDT*>(pmm.translate_to_hhdm(xsdp->rsdt_address));
		if (rsdt->header.checksum_correct()) {
			Logger::log_info("ACPI: Setting up ACPI from the RSDT.\n");
			setup_acpi_from_rsdt(rsdt);
			return;
		} else {
			Logger::log_error("ACPI: RSDT is invalid.\n");
			failure = true;
		}
	}

	if (failure) {
		panic("ACPI: Failed to set up ACPI, root table is invalid.\n");
	}

	panic("ACPI: Failed to set up ACPI, unknown revision: %d.\n", xsdp->revision);
}

// We need these to be loaded or else we die
static bool madt_loaded = false;

static void setup_acpi_from_xsdt(XSDT* xsdt) {
	Logger::log_info("ACPI: XSDT Entries:\n");

	bool any_failures = false;

	Logger::increase_indent();
	for (uint64_t i = 0; i < xsdt->entry_count(); i++) {
		if (!parse_root_entry(xsdt->entries()[i]))
			any_failures = true;
	}
	Logger::decrease_indent();

	if (!madt_loaded) {
		panic("ACPI: Failed to load the MADT.\n");
	}

	if (any_failures) {
		panic("ACPI: XSDT contains invalid entries.\n");
	}
}

static void setup_acpi_from_rsdt(RSDT* rsdt) {
	Logger::log_info("ACPI: RSDT Entries:\n");

	bool any_failures = false;

	Logger::increase_indent();
	for (uint32_t i = 0; i < rsdt->entry_count(); i++) {
		if (!parse_root_entry(rsdt->entries()[i]))
			any_failures = true;
	}
	Logger::decrease_indent();

	if (!madt_loaded) {
		panic("ACPI: Failed to load the MADT.\n");
	}

	if (any_failures) {
		panic("ACPI: RSDT contains invalid entries.\n");
	}
}

static bool parse_root_entry(uint64_t entry) {
	SDTHeader* entry_header = reinterpret_cast<SDTHeader*>(pmm.translate_to_hhdm(entry));

	char displayed_signature[5];
	char displayed_oem_id[7];
	char displayed_oem_table_id[8];

	memcpy(displayed_signature, entry_header->signature, 4);
	memcpy(displayed_oem_id, entry_header->oem_id, 6);
	memcpy(displayed_oem_table_id, entry_header->oem_table_id, 8);

	displayed_signature[4] = 0;
	displayed_oem_id[6] = 0;
	displayed_oem_table_id[7] = 0;

	Logger::log_info("- 0x%X: %s rev. %d, OEM: %s rev. %d, OEM Table ID: %s", entry, displayed_signature, entry_header->revision, displayed_oem_id, entry_header->oem_revision, displayed_oem_table_id);

	if (!entry_header->checksum_correct()) {
		Logger::kprintf(" - invalid.\n");
		return false;
	}

	if (memcmp(entry_header->signature, "APIC", 4) == 0) {
		Logger::kprintf("\n");
		Logger::increase_indent();
		ACPI::Interrupts::init(reinterpret_cast<MADT::MADTHeader*>(entry_header));
		madt_loaded = true;
		Logger::decrease_indent();
	} else {
		// Skip any unused entries
		Logger::kprintf(" - skip\n");
	}

	return true;
}

}
