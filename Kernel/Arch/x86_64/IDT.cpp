#include <Arch/x86_64/IDT.hpp>
#include <Core/Panic.hpp>

namespace Interrupts::IDT {

DynamicIRQEntry dynamic_handlers[DYNAMIC_VECTOR_COUNT];

static uint8_t next_dynamic_vector = FIRST_DYNAMIC_VECTOR;

uint8_t allocate_dynamic_vector() {
	if (next_dynamic_vector >= FIRST_DYNAMIC_VECTOR && next_dynamic_vector <= LAST_DYNAMIC_VECTOR) {
		return next_dynamic_vector++;
	}

	panic("Run out of dynamic interrupt vectors.\n");
}

void set_dynamic_handler(uint8_t vector, void* object, void (*handler)(void*, Registers*)) {
	if (next_dynamic_vector < FIRST_DYNAMIC_VECTOR || next_dynamic_vector > LAST_DYNAMIC_VECTOR) {
		panic("set_dynamic_handler() called with vector %d, outside of dynamic range %d-%d.\n", vector, FIRST_DYNAMIC_VECTOR, LAST_DYNAMIC_VECTOR);
	}

	dynamic_handlers[vector - FIRST_DYNAMIC_VECTOR] = {
		object,
		handler
	};
}

}
