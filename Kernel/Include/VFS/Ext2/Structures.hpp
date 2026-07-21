#pragma once

#include <cstdint>

namespace VFS::Ext2 {

enum class OptionalFeatureFlags : uint32_t {
	PreallocateDirectoryBlocks = 0x1,
	AFSServerInodesExist = 0x2,
	JournalPresent = 0x4,
	InodeExtendedAttributes = 0x8,
	FSCanResize = 0x10,
	DirectoriesUseHashIndex = 0x20
};

enum class RequiredFeatureFlags : uint32_t {
	CompressionIsUsed = 0x1,
	DirectoryEntriesHaveTypeField = 0x2,
	JournalNeedsReplay = 0x4,
	JournalDeviceUsed = 0x8
};

enum class ReadOnlyFeatureFlags : uint32_t {
	SparseSuperblocksAndGroupDescriptorTables = 0x1,
	FilesSize64bit = 0x2,
	BinaryTreeDirectoryContents = 0x4
};

struct Superblock {
	uint32_t total_inode_count;
	uint32_t total_block_count;
	uint32_t reserved_block_count;
	uint32_t total_unallocated_blocks;
	uint32_t total_unallocated_inodes;
	uint32_t starting_block_number;
	uint32_t log2_block_size;
	uint32_t log2_fragment_size;
	uint32_t blocks_per_block_group;
	uint32_t fragments_per_block_group;
	uint32_t inodes_per_block_group;
	uint32_t last_mount_time;
	uint32_t last_write_time;
	uint16_t mounts_since_fsck;
	uint16_t mounts_before_fsck;
	uint16_t signature;
	uint16_t state;
	uint16_t error_behaviour;
	uint16_t version_minor;
	uint32_t last_fsck_time;
	uint32_t fsck_time_interval;
	uint32_t creator_os_id;
	uint32_t version_major;
	uint16_t reserved_blocks_uid;
	uint16_t reserved_blocks_gid;

	uint64_t block_size() const {
		return 1024 << log2_block_size;
	}

	uint64_t fragment_size() const {
		return 1024 << log2_fragment_size;
	}

	bool is_extended() const {
		return version_major >= 1;
	}

	// Extended fields
	struct {
		uint32_t first_nonreserved_inode_number;
		uint16_t inode_structure_size;
		uint16_t this_superblocks_block_group;
		uint32_t optional_features_present;
		uint32_t required_features_present;
		uint32_t write_features_present;
		uint8_t fs_ide[16];
		char volumne_name[16];
		char last_mountpoint[64];
		uint32_t compression_algorithms_used;
		uint8_t preallocated_blocks_files;
		uint8_t preallocated_blocks_directories;
		uint16_t unused;
		uint8_t journal_id[16];
		uint32_t journal_inode;
		uint32_t journal_device;
		uint32_t orphan_inode_list_head;

		// Bytes 236-1023 inclusive are unused
	} extended;

	uint32_t first_nonreserved_inode_number() const {
		return is_extended() ? extended.first_nonreserved_inode_number : 11;
	}

	uint32_t inode_structure_size() const {
		return is_extended() ? extended.inode_structure_size : 128;
	}
} __attribute__((packed));

struct BlockGroupDescriptor {
	uint32_t block_bitmap_block;
	uint32_t inode_bitmap_block;
	uint32_t inode_table_starting_block;
	uint16_t free_blocks;
	uint16_t free_inodes;
	uint16_t directory_count;

	uint8_t unused[14];
} __attribute__((packed));

static_assert(sizeof(BlockGroupDescriptor) == 32);

struct Inode {
	uint16_t type_and_permissions;
	uint16_t user_id;
	uint32_t size_low32;
	uint32_t access_time;
	uint32_t create_time;
	uint32_t modify_time;
	uint32_t delete_time;
	uint16_t group_id;
	uint16_t hardlink_count;
	uint32_t disk_sector_count;
	uint32_t flags;
	uint32_t os_specific1;
	uint32_t direct_blocks[12];
	uint32_t singly_indirect_block;
	uint32_t doubly_indirect_block;
	uint32_t triply_indirect_block;
	uint32_t generation_number;
	uint32_t extended_attribute_block;
	uint32_t size_high32_directory_acl;
	uint32_t fragment_block;
	uint8_t os_specific2[12];
} __attribute__((packed));

}
