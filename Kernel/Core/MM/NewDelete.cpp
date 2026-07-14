#include <Core/MM/KernelHeap.hpp>
#include <cstddef>

void* operator new(size_t size) {
	return kernel_heap.alloc(size);
}

void* operator new[](size_t size) {
	return kernel_heap.alloc(size);
}

void operator delete(void* ptr) noexcept {
	kernel_heap.free(ptr);
}

void operator delete[](void* ptr) noexcept {
	kernel_heap.free(ptr);
}
void operator delete(void* ptr, size_t) noexcept {
	kernel_heap.free(ptr);
}

void operator delete[](void* ptr, size_t) noexcept {
	kernel_heap.free(ptr);
}

void* operator new(size_t, void* ptr) noexcept {
	return ptr;
}

void operator delete(void*, void*) noexcept {
}
