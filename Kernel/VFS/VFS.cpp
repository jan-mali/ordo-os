#include <Core/memory.hpp>
#include <Logger/Logger.hpp>
#include <Logger/kstdio.hpp>
#include <Utility/String.hpp>
#include <VFS/FileSystem.hpp>
#include <VFS/OpenFile.hpp>
#include <VFS/VFS.hpp>
#include <VFS/VFSNode.hpp>
#include <cstddef>

namespace VFS {

FileSystem* VFS::root_fs;
VFSNode* VFS::root_node;
Vector<MountPoint> VFS::mounts;

void VFS::mount_root_filesystem(FileSystem* fs) {
	Logger::log_info("VFS: Mounting the root filesystem.\n");

	root_fs = fs;
	root_node = fs->root();
}

bool VFS::mount(FileSystem* fs, const char* path) {
	VFSNode* target = resolve(path);
	return mount(fs, target);
}

bool VFS::mount(VFSNode* node, const char* path) {
	VFSNode* target = resolve(path);
	return mount(node, target);
}

bool VFS::mount(FileSystem* fs, VFSNode* target) {
	// TODO: Add error checking
	mounts.push_back({ .location = target, .fs = fs, .root = fs->root() });
	return true;
}

bool VFS::mount(VFSNode* node, VFSNode* target) {
	// TODO: Add error checking
	mounts.push_back({ .location = target, .fs = node->filesystem(), .root = node });
	return true;
}

VFSNode* VFS::resolve(const char* path) {
	char component[512];
	const char* p = path;

	VFSNode* current = root_node;

	VFSNode* parents[256];
	size_t parent_count = 0;

	while (*p) {
		// Skip consecutive '/'
		while (*p == '/')
			++p;

		if (*p == '\0')
			break;

		const char* component_begin = p;

		// Find end of component
		while (*p && *p != '/')
			++p;

		const char* component_end = p;

		size_t component_length = component_end - component_begin;

		memcpy(component, component_begin, component_length);
		component[component_length] = '\0';

		Logger::log_info("VFS::resolve: %16s", component);

		if (strcmp(component, ".") == 0) {
			Logger::kprintf(" - current directory\n");
			continue;
		}

		if (strcmp(component, "..") == 0) {
			Logger::kprintf(" - parent directory\n");

			if (parent_count > 0) {
				current = parents[--parent_count];
			} else {
				// Already at root, stay there
				current = root_node;
			}

			continue;
		}

		VFSNode* next = current->lookup(component);

		if (!next) {
			Logger::kprintf(" - not found\n");
			return nullptr;
		}

		// Save current node before moving down
		if (parent_count >= sizeof(parents) / sizeof(parents[0])) {
			Logger::log_error("VFS::resolve: path too deep\n");
			return nullptr;
		}

		parents[parent_count++] = current;
		current = next;

		bool is_mount = false;

		for (MountPoint mount : mounts) {
			if (current == mount.location) {
				Logger::kprintf(" - mountpoint\n");
				current = mount.root;
				is_mount = true;
				break;
			}
		}

		if (is_mount) {
			continue;
		}

		Logger::kprintf(" - found\n");
	}

	return current;
}

OpenFile* VFS::open(const char* path) {
	VFSNode* node = resolve(path);

	if (!node) {
		return nullptr;
	}

	if (node->node_type() != VFSNodeType::File) {
		return nullptr;
	}

	OpenFile* file = new OpenFile(node);

	return file;
}

void VFS::close(OpenFile* file) {
	delete file;
}

void debug_list_directory(VFSNode* dir) {
	DirectoryEntry entry;

	for (uint64_t i = 0; dir->readdir(i, &entry); i++) {
		char entry_type;

		switch (entry.type) {
		case VFSNodeType::File:
			entry_type = 'F';
			break;
		case VFSNodeType::Directory:
			entry_type = 'D';
			break;
		default:
			entry_type = 'U';
			break;
		}

		Logger::log_info("%2d. [%c] %s\n", i + 1, entry_type, entry.name);
	}
}

}
