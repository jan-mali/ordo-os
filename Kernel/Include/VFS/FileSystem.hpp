#pragma once

#include <VFS/VFSNode.hpp>

namespace VFS {

class FileSystem {
public:
	virtual ~FileSystem() = default;

	virtual VFSNode* root() = 0;

private:
};

}
