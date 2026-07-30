#pragma once

#include <Core/Result.hpp>
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
	static Result<void> mount_root_filesystem(FileSystem* fs);
	static Result<void> mount(FileSystem* fs, const char* path);
	static Result<void> mount(VFSNode* node, const char* path);
	static Result<void> mount(FileSystem* fs, VFSNode* target);
	static Result<void> mount(VFSNode* node, VFSNode* target);
	static Result<VFSNode*> resolve(const char* path);

	static Result<OpenFile*> open(const char* path);
	static Result<void> close(OpenFile* file);

private:
	static FileSystem* root_fs;
	static VFSNode* root_node;
	static Vector<MountPoint> mounts;
};

void debug_list_directory(VFSNode* dir);

}
