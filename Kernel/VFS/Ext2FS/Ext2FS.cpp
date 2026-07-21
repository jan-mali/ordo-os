#include <Logger/Logger.hpp>
#include <VFS/Ext2/Ext2FS.hpp>
#include <VFS/Ext2/Ext2Node.hpp>
#include <VFS/Ext2/Structures.hpp>

namespace VFS::Ext2 {

VFSNode* Ext2FS::root() {
	return root_directory;
}

Ext2FS* Ext2FS::mount(Drivers::BlockDevice* device) {
	Ext2FS* fs = new Ext2FS();

	fs->device = device;

	if (!device->read_bytes(1024, sizeof(fs->superblock), reinterpret_cast<uint8_t*>(&fs->superblock))) {
		Logger::log_error("Ext2::mount: Failed to read the superblock\n");

		delete fs;
		return nullptr;
	}

	if (fs->superblock.signature != 0xef53) {
		Logger::log_error("Ext2::mount: Superblock signature is invalid\n");

		delete fs;
		return nullptr;
	}

	fs->block_size = fs->superblock.block_size();

	uint64_t block_groups_from_inodes = (fs->superblock.total_inode_count + fs->superblock.inodes_per_block_group - 1) / fs->superblock.inodes_per_block_group;
	uint64_t block_groups_from_blocks = (fs->superblock.total_block_count + fs->superblock.blocks_per_block_group - 1) / fs->superblock.blocks_per_block_group;

	if (block_groups_from_inodes != block_groups_from_blocks) {
		Logger::log_error("Ext2::mount: Block group counts are contradictory (%lld vs %lld).\n", block_groups_from_inodes, block_groups_from_blocks);

		delete fs;
		return nullptr;
	}

	fs->block_group_count = block_groups_from_inodes;

	if (fs->superblock.is_extended()) {
		if (fs->superblock.extended.required_features_present) {
			Logger::log_error("Ext2::mount: No support for required features: 0x%08x.\n", fs->superblock.extended.required_features_present);

			delete fs;
			return nullptr;
		}

		if (fs->superblock.extended.optional_features_present) {
			Logger::log_warning("Ext2::mount: No support for optional features: 0x%08x.\n", fs->superblock.extended.optional_features_present);
		}

		if (fs->superblock.extended.write_features_present) {
			Logger::log_error("Ext2::mount: No support for required write-only features: 0x%08x.\n", fs->superblock.extended.write_features_present);

			// TODO: Mark the filesystem as read-only
			Logger::log_debug("Ext2::mount: TODO: Mount the filesystem as read-only.\n");
		}
	}

	fs->bgdt_block = fs->superblock.starting_block_number + 1;
	uint64_t bgdt_address = fs->block_address(fs->bgdt_block);

	Logger::log_info("Ext2::mount: Successfully parsed the superblock.\n");
	Logger::log_info("Ext2::mount: \tInodes used: %d/%d.\n", fs->superblock.total_inode_count - fs->superblock.total_unallocated_inodes, fs->superblock.total_inode_count);
	Logger::log_info("Ext2::mount: \tBlocks used: %d/%d.\n", fs->superblock.total_block_count - fs->superblock.total_unallocated_blocks, fs->superblock.total_block_count);
	Logger::log_info("Ext2::mount: \tBlock size: %lld.\n", fs->block_size);
	Logger::log_info("Ext2::mount: \tBlock group count: %lld.\n", fs->block_group_count);
	Logger::log_info("Ext2::mount: \tInodes/group: %d.\n", fs->superblock.inodes_per_block_group);
	Logger::log_info("Ext2::mount: \tBlocks/group: %d.\n", fs->superblock.blocks_per_block_group);
	Logger::log_info("Ext2::mount: \tSuperblock block (starting block): %d.\n", fs->superblock.starting_block_number);
	Logger::log_info("Ext2::mount: \tBGDT's block & address: %d; 0x%llX.\n", fs->bgdt_block, bgdt_address);

	BlockGroupDescriptor* bgdt = new BlockGroupDescriptor[fs->block_group_count];

	if (!device->read_bytes(bgdt_address, fs->block_group_count * sizeof(BlockGroupDescriptor), reinterpret_cast<uint8_t*>(bgdt))) {
		Logger::log_error("Ext2::mount: Failed to read the Block Group Descriptor Table\n");

		delete[] bgdt;
		delete fs;
		return nullptr;
	}

	fs->bgdt = bgdt;

	fs->root_directory = new Ext2Node(fs, 2);

	return fs;
}

uint64_t Ext2FS::block_address(uint32_t block) {
	return static_cast<uint64_t>(block) * block_size;
}

Ext2Node* Ext2FS::get_inode(uint32_t inode_number) {
	auto existing = inode_cache.get(inode_number);

	if (existing != nullptr)
		return *existing;

	Ext2Node* node = new Ext2Node(this, inode_number);

	inode_cache.insert(inode_number, node);

	return node;
}

}
