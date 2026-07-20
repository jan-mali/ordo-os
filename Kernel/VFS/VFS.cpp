#include <Core/memory.hpp>
#include <Logger/Logger.hpp>
#include <Logger/kstdio.hpp>
#include <Utility/String.hpp>
#include <VFS/FileSystem.hpp>
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
			Logger::kprintf("- current directory\n");
			continue;
		}

		if (strcmp(component, "..") == 0) {
			Logger::kprintf("- parent directory\n");
			Logger::log_error("Parent directories are not yet supported :'(\n");
			return nullptr;
		}

		current = current->lookup(component);
		if (!current) {
			Logger::kprintf(" - not found\n");
			return nullptr;
		}

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

	// TODO: Resolve paths
	return current;
}

}
