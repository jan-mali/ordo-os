#pragma once

#include <cstdint>

namespace VFS {

class FileSystem;

enum class VFSNodeType {
	File,
	Directory
};

struct DirectoryEntry {
	char name[256];
	VFSNodeType type;
};

class VFSNode {
public:
	VFSNode() = default;
	virtual ~VFSNode() = default;

	// File services
	virtual uint64_t read(uint64_t offset, uint64_t bytes, uint8_t* buffer) = 0;
	virtual uint64_t write(uint64_t offset, uint64_t bytes, uint8_t* buffer) = 0;
	virtual uint64_t size() = 0;

	// Directory services
	virtual VFSNode* lookup(const char* name) = 0;
	virtual VFSNode* mkdir(const char* name) = 0;
	virtual VFSNode* create(const char* name) = 0;
	virtual uint64_t entry_count() = 0;
	virtual bool readdir(uint64_t index, DirectoryEntry* entry) = 0;

	virtual FileSystem* filesystem() = 0;

	VFSNodeType node_type() const { return type; }

	// Don't allow copying of VFSNode
	VFSNode(const VFSNode&) = delete;
	VFSNode& operator=(const VFSNode&) = delete;

protected:
	VFSNodeType type;
};

}
