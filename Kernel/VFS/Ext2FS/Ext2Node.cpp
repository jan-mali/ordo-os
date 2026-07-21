#include <Core/memory.hpp>
#include <Logger/Logger.hpp>
#include <Utility/String.hpp>
#include <VFS/Ext2/Ext2Node.hpp>
#include <VFS/VFSNode.hpp>

namespace VFS::Ext2 {

Ext2Node::Ext2Node(Ext2FS* fs, uint32_t inode_number)
	: fs(fs) {
	block_group = (inode_number - 1) / fs->superblock.inodes_per_block_group;
	bg_index = (inode_number - 1) % fs->superblock.inodes_per_block_group;

	uint32_t inode_table_start = fs->bgdt[block_group].inode_table_starting_block;
	uint32_t containing_block = (bg_index * fs->superblock.inode_structure_size()) / fs->block_size;
	uint32_t block_offset_bytes = (bg_index * fs->superblock.inode_structure_size()) % fs->block_size;

	uint32_t inode_structure_address = fs->block_address(inode_table_start + containing_block) + block_offset_bytes;

	// FIXME: We assume this succeeds as we have no way of reporting errors :(
	fs->device->read_bytes(inode_structure_address, sizeof(Inode), reinterpret_cast<uint8_t*>(&inode));

	uint8_t ext2_type = (inode.type_and_permissions >> 12) & 0xF;

	switch (ext2_type) {
	case 0x8:
		type = VFSNodeType::File;
		break;
	case 0x4:
		type = VFSNodeType::Directory;
		break;
	default:
		Logger::log_error("Ext2: Cannot represent inode with type %d\n", static_cast<uint8_t>(ext2_type));
		break;
	}
}

uint64_t Ext2Node::read(uint64_t offset, uint64_t bytes, uint8_t* buffer) {
	if (offset >= inode.size_low32) {
		return 0;
	}

	if (offset + bytes > inode.size_low32) {
		bytes = inode.size_low32 - offset;
	}

	uint64_t bytes_read = 0;

	while (bytes_read < bytes) {
		uint64_t current_offset = offset + bytes_read;

		uint32_t block_index = current_offset / fs->block_size;
		uint32_t block_offset = current_offset % fs->block_size;

		uint32_t block;

		if (!get_data_block(block_index, &block)) {
			break;
		}

		uint64_t chunk = fs->block_size - block_offset;

		if (chunk > bytes - bytes_read)
			chunk = bytes - bytes_read;

		// Sparse block
		if (block == 0) {
			memset(buffer + bytes_read, 0, chunk);
			bytes_read += chunk;
			continue;
		}

		uint64_t address = fs->block_address(block);

		if (!fs->device->read_bytes(address + block_offset, chunk, buffer + bytes_read)) {
			break;
		}

		bytes_read += chunk;
	}

	return bytes_read;
}

uint64_t Ext2Node::write(uint64_t offset, uint64_t bytes, uint8_t* buffer) {
	(void)offset;
	(void)bytes;
	(void)buffer;

	// TODO: Add write support

	return 0;
}

uint64_t Ext2Node::size() {
	// TODO: Support large files
	return inode.size_low32;
}

Ext2Node* Ext2Node::lookup(const char* name) {
	if (type != VFSNodeType::Directory)
		return nullptr;

	uint64_t count = entry_count();

	char entry_name[256];
	Ext2DirectoryEntry raw_entry;

	for (uint64_t i = 0; i < count; i++) {
		if (!get_directory_entry(i, &raw_entry, entry_name, sizeof(entry_name))) {
			return nullptr;
		}

		if (strcmp(entry_name, name) == 0) {
			return fs->get_inode(raw_entry.inode);
		}
	}

	return nullptr;
}

Ext2Node* Ext2Node::mkdir(const char* name) {
	(void)name;

	// TODO: Add write support

	return nullptr;
}

Ext2Node* Ext2Node::create(const char* name) {
	(void)name;

	// TODO: Add write support

	return nullptr;
}

uint64_t Ext2Node::entry_count() {
	if (type != VFSNodeType::Directory)
		return 0;

	uint64_t offset = 0;
	uint64_t count = 0;

	Ext2DirectoryEntry entry;

	while (offset < inode.size_low32) {
		if (read(offset, sizeof(entry), reinterpret_cast<uint8_t*>(&entry)) != sizeof(entry)) {
			break;
		}

		if (entry.entry_length < sizeof(entry)) {
			break;
		}

		if (entry.inode != 0)
			count++;

		offset += entry.entry_length;
	}

	return count;
}

bool Ext2Node::readdir(uint64_t index, DirectoryEntry* entry) {
	Ext2DirectoryEntry ext2_entry;
	char name[256];

	if (!get_directory_entry(index, &ext2_entry, name, sizeof(name))) {
		return false;
	}

	strncpy(entry->name, name, sizeof(entry->name));
	entry->name[sizeof(entry->name) - 1] = '\0';

	Ext2Node* node = fs->get_inode(ext2_entry.inode);

	if (!node) {
		return false;
	}

	entry->type = node->type;

	return true;
}

FileSystem* Ext2Node::filesystem() {
	return fs;
}

bool Ext2Node::get_directory_entry(uint64_t index, Ext2DirectoryEntry* result, char* name_buffer, size_t name_buffer_size) {
	uint64_t offset = 0;
	uint64_t current = 0;

	Ext2DirectoryEntry header;

	while (offset < inode.size_low32) {
		if (read(offset, sizeof(Ext2DirectoryEntry), reinterpret_cast<uint8_t*>(&header)) != sizeof(Ext2DirectoryEntry))
			return false;

		// Corrupted directory
		if (header.entry_length < sizeof(Ext2DirectoryEntry))
			return false;

		// Skip unused entries
		if (header.inode != 0) {
			if (current == index) {
				if (header.name_length >= name_buffer_size) {
					return false;
				}

				if (read(offset + sizeof(Ext2DirectoryEntry), header.name_length, reinterpret_cast<uint8_t*>(name_buffer)) != header.name_length) {
					return false;
				}

				name_buffer[header.name_length] = '\0';

				*result = header;
				return true;
			}

			current++;
		}

		offset += header.entry_length;
	}

	return false;
}

bool Ext2Node::get_data_block(uint32_t index, uint32_t* block) {
	// Direct blocks
	if (index < 12) {
		*block = inode.direct_blocks[index];
		return true;
	}

	// Single indirect
	index -= 12;

	if (index >= fs->block_size / sizeof(uint32_t)) {
		return false;
	}

	if (inode.singly_indirect_block == 0) {
		*block = 0;
		return true;
	}

	uint32_t indirect_block[fs->block_size / sizeof(uint32_t)];

	if (!fs->device->read_bytes(fs->block_address(inode.singly_indirect_block), fs->block_size, reinterpret_cast<uint8_t*>(indirect_block))) {
		return false;
	}

	*block = indirect_block[index];
	return true;
}

};
