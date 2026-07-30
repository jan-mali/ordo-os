#pragma once

#include <VFS/Ext2/Ext2FS.hpp>
#include <VFS/Ext2/Structures.hpp>
#include <VFS/VFSNode.hpp>

namespace VFS::Ext2 {

struct Ext2DirectoryEntry {
	uint32_t inode;
	uint16_t entry_length;
	uint8_t name_length;
	uint8_t file_type;
	char name[];
};

enum class Ext2DirectoryEntryFileType : uint8_t {
	Unknown = 0,
	RegularFile = 1,
	Directory = 2,
	CharDevice = 3,
	BlockDevice = 4,
	FIFO = 5,
	Socket = 6,
	SymbolicLink = 7
};

class Ext2Node : public VFSNode {
public:
	Ext2Node(Ext2FS* fs, uint32_t inode_number);
	virtual ~Ext2Node() = default;

	// File services
	Result<uint64_t> read(uint64_t offset, uint64_t bytes, uint8_t* buffer) override;
	Result<uint64_t> write(uint64_t offset, uint64_t bytes, uint8_t* buffer) override;
	Result<uint64_t> size() override;

	// Directory services
	Result<VFSNode*> lookup(const char* name) override;
	Result<VFSNode*> mkdir(const char* name) override;
	Result<VFSNode*> create(const char* name) override;
	Result<uint64_t> entry_count() override;
	Result<void> readdir(uint64_t index, DirectoryEntry* entry) override;

	FileSystem* filesystem() override;

	VFSNodeType node_type() const { return type; }

private:
	Result<void> get_directory_entry(uint64_t index, Ext2DirectoryEntry* result, char* name_buffer, size_t name_buffer_size);
	bool get_data_block(uint32_t index, uint32_t* block);

	Result<uint64_t> inode_read(uint64_t offset, uint64_t bytes, uint8_t* buffer);
	Result<uint64_t> inode_write(uint64_t offset, uint64_t bytes, uint8_t* buffer);

	Ext2FS* fs;

	uint32_t block_group;
	uint32_t bg_index;
	uint32_t containing_block;

	Inode inode;
};

}
