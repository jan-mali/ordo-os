#pragma once

#include <Utility/Vector.hpp>
#include <VFS/FileSystem.hpp>
#include <VFS/VFSNode.hpp>

namespace VFS::TmpFS {

class TmpFSNode : public VFSNode {
	friend class TmpFS;

public:
	struct TmpFSDirectoryEntry {
		const char* name;
		TmpFSNode* node;
	};

	virtual ~TmpFSNode() = default;

	// File services
	virtual uint64_t read(uint64_t offset, uint64_t bytes, uint8_t* buffer) override;
	virtual uint64_t write(uint64_t offset, uint64_t bytes, uint8_t* buffer) override;
	virtual uint64_t size() override;

	// Directory services
	virtual TmpFSNode* lookup(const char* name) override;
	virtual TmpFSNode* mkdir(const char* name) override;
	virtual TmpFSNode* create(const char* name) override;
	virtual uint64_t entry_count() override;
	virtual bool readdir(uint64_t index, DirectoryEntry* entry) override;

	virtual FileSystem* filesystem() override {
		return fs;
	}

	void dump(int indent = 0);

private:
	// File data
	uint64_t file_size;
	uint8_t* data;

	// Directory data
	Vector<TmpFSDirectoryEntry> entries;

	FileSystem* fs;
};

class TmpFS : public FileSystem {
public:
	TmpFS();
	virtual ~TmpFS() = default;

	virtual TmpFSNode* root() override;

private:
	TmpFSNode* root_directory;
};

}
