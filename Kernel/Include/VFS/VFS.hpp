#pragma once

#include <Utility/Vector.hpp>
#include <VFS/FileSystem.hpp>
#include <VFS/OpenFile.hpp>
#include <VFS/VFSNode.hpp>

namespace VFS {

struct MountPoint {
	VFSNode* location;
	FileSystem* fs;
	VFSNode* root;
};

class VFS {
public:
	static void mount_root_filesystem(FileSystem* fs);
	static bool mount(FileSystem* fs, const char* path);
	static bool mount(VFSNode* node, const char* path);
	static bool mount(FileSystem* fs, VFSNode* target);
	static bool mount(VFSNode* node, VFSNode* target);
	static VFSNode* resolve(const char* path);

	static OpenFile* open(const char* path);
	static void close(OpenFile* file);

private:
	static FileSystem* root_fs;
	static VFSNode* root_node;
	static Vector<MountPoint> mounts;
};

void debug_list_directory(VFSNode* dir);

}
