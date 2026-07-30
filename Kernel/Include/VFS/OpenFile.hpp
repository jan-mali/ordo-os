#pragma once

#include <Core/Result.hpp>
#include <VFS/VFSNode.hpp>

namespace VFS {

struct OpenFile {
	OpenFile(VFSNode* node)
		: node(node)
		, offset(0)
		, flags(0) {}

	VFSNode* node;

	uint64_t offset;

	// TODO
	uint32_t flags;

	Result<uint64_t> read(uint64_t bytes, uint8_t* buffer) {
		uint64_t result = TRY(node->read(offset, bytes, buffer));
		offset += result;
		return result;
	}

	Result<uint64_t> write(uint64_t bytes, uint8_t* buffer) {
		uint64_t result = TRY(node->write(offset, bytes, buffer));
		offset += result;
		return result;
	}
};

}
