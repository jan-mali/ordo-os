#include <Core/Result.hpp>
#include <Core/memory.hpp>
#include <Logger/Logger.hpp>
#include <Logger/kstdio.hpp>
#include <Utility/String.hpp>
#include <VFS/TmpFS/TmpFS.hpp>
#include <VFS/VFSNode.hpp>

namespace VFS::TmpFS {

Result<uint64_t> TmpFSNode::read(uint64_t offset, uint64_t bytes, uint8_t* buffer) {
	if (type != VFSNodeType::File) {
		return Error {
			.domain = ErrorDomain::FileSystem,
			.code = ErrorCode::IsDirectory
		};
	}

	if (offset + bytes > file_size) {
		bytes = file_size - offset;
	}

	memcpy(buffer, data + offset, bytes);

	return bytes;
}

Result<uint64_t> TmpFSNode::write(uint64_t offset, uint64_t bytes, uint8_t* buffer) {
	if (type != VFSNodeType::File) {
		return Error {
			.domain = ErrorDomain::FileSystem,
			.code = ErrorCode::IsDirectory
		};
	}

	if (offset + bytes > file_size) {
		Logger::log_warning("TmpFS: Attempted to write bytes past the end of the file\n");
		Logger::log_debug("TmpFS: TODO: Allow the files to grow in size\n");
		bytes = file_size - offset;
	}

	memcpy(data + offset, buffer, bytes);

	return bytes;
}

Result<uint64_t> TmpFSNode::size() {
	if (type != VFSNodeType::File) {
		return Error {
			.domain = ErrorDomain::FileSystem,
			.code = ErrorCode::IsDirectory
		};
	}

	return file_size;
}

Result<uint64_t> TmpFSNode::entry_count() {
	if (type != VFSNodeType::Directory) {
		return Error {
			.domain = ErrorDomain::FileSystem,
			.code = ErrorCode::NotDirectory
		};
	}

	return entries.size();
}

Result<VFSNode*> TmpFSNode::lookup(const char* name) {
	if (type != VFSNodeType::Directory) {
		return Error {
			.domain = ErrorDomain::FileSystem,
			.code = ErrorCode::NotDirectory
		};
	}

	for (auto file : entries) {
		if (strcmp(file.name, name) == 0) {
			return file.node;
		}
	}

	return Errors::not_found(ErrorDomain::FileSystem);
}

Result<VFSNode*> TmpFSNode::mkdir(const char* name) {
	if (type != VFSNodeType::Directory) {
		return Error {
			.domain = ErrorDomain::FileSystem,
			.code = ErrorCode::NotDirectory
		};
	}

	for (auto file : entries) {
		if (strcmp(file.name, name) == 0) {
			return Error {
				.domain = ErrorDomain::FileSystem,
				.code = ErrorCode::AlreadyExists
			};
		}
	}

	TmpFSNode* directory = new TmpFSNode();
	directory->type = VFSNodeType::Directory;
	directory->fs = fs;

	entries.push_back({ .name = strdup(name), .node = directory });

	return directory;
}

Result<VFSNode*> TmpFSNode::create(const char* name) {
	if (type != VFSNodeType::Directory) {
		return Error {
			.domain = ErrorDomain::FileSystem,
			.code = ErrorCode::NotDirectory
		};
	}

	for (auto file : entries) {
		return Error {
			.domain = ErrorDomain::FileSystem,
			.code = ErrorCode::AlreadyExists
		};
	}

	TmpFSNode* file = new TmpFSNode();
	file->type = VFSNodeType::File;
	file->fs = fs;

	file->file_size = 4096;
	file->data = new uint8_t[4096];

	entries.push_back({ .name = strdup(name), .node = file });

	return file;
}

Result<void> TmpFSNode::readdir(uint64_t index, DirectoryEntry* entry) {
	if (type != VFSNodeType::Directory) {
		return Error {
			.domain = ErrorDomain::FileSystem,
			.code = ErrorCode::NotDirectory
		};
	}

	if (index >= entries.size()) {
		return Error {
			.domain = ErrorDomain::FileSystem,
			.code = ErrorCode::NotFound
		};
	}

	TmpFSDirectoryEntry e = entries[index];

	size_t name_length = strlen(e.name);
	if (name_length > sizeof(entry->name)) {
		name_length = sizeof(entry->name);
	}

	memcpy(entry->name, e.name, name_length);
	entry->name[name_length] = '\0';

	entry->type = e.node->type;

	return {};
}

void TmpFSNode::dump(int indent) {

	switch (type) {
	case VFSNodeType::File: {
		Logger::kprintf("[FILE]\n");
		break;
	}
	case VFSNodeType::Directory: {
		Logger::kprintf("[DIR]\n");
		for (TmpFSDirectoryEntry entry : entries) {
			for (int i = 0; i < indent; i++) {
				Logger::kprintf(" ");
			}
			Logger::kprintf("      %s ", entry.name);
			entry.node->dump(indent + 6);
		}
		break;
	}
	default: {
		Logger::kprintf("[UNKNOWN]\n");
		break;
	}
	}
}

TmpFS::TmpFS() {
	root_directory = new TmpFSNode();
	root_directory->type = VFSNodeType::Directory;
	root_directory->fs = this;
}

TmpFSNode* TmpFS::root() {
	return root_directory;
}

}
