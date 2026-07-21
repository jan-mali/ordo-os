#pragma once

#include <Drivers/BlockDevice.hpp>
#include <Utility/Map.hpp>
#include <VFS/Ext2/Structures.hpp>
#include <VFS/FileSystem.hpp>

namespace VFS::Ext2 {

class Ext2Node;

class Ext2FS : public FileSystem {
	friend class Ext2Node;

public:
	VFSNode* root() override;

	static Ext2FS* mount(Drivers::BlockDevice* device);

	uint64_t block_address(uint32_t block);

	Ext2Node* get_inode(uint32_t inode_number);

private:
	Drivers::BlockDevice* device;
	Superblock superblock;

	uint64_t block_size;
	uint64_t block_group_count;

	// Block Group Descriptor Table's block number
	uint32_t bgdt_block;

	BlockGroupDescriptor* bgdt;

	Ext2Node* root_directory;

	Map<uint32_t, Ext2Node*> inode_cache;
};

}
