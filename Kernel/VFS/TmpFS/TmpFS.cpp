#include <Core/memory.hpp>
#include <Logger/Logger.hpp>
#include <Logger/kstdio.hpp>
#include <Utility/String.hpp>
#include <VFS/TmpFS/TmpFS.hpp>
#include <VFS/VFSNode.hpp>

namespace VFS::TmpFS {

uint64_t TmpFSNode::read(uint64_t offset, uint64_t bytes, uint8_t* buffer) {
	if (type != VFSNodeType::File) {
		Logger::log_error("TmpFS: Attempted to read from a Directory node.\n");
		return 0;
	}

	if (offset + bytes > file_size) {
		bytes = file_size - offset;
	}

	memcpy(buffer, data + offset, bytes);

	return bytes;
}

uint64_t TmpFSNode::write(uint64_t offset, uint64_t bytes, uint8_t* buffer) {
	if (type != VFSNodeType::File) {
		Logger::log_error("TmpFS: Attempted to read from a Directory node.\n");
		return 0;
	}

	if (offset + bytes > file_size) {
		Logger::log_warning("TmpFS: Attempted to write bytes past the end of the file\n");
		Logger::log_debug("TmpFS: TODO: Allow the files to grow in size\n");
		bytes = file_size - offset;
	}

	memcpy(data + offset, buffer, bytes);

	return bytes;
}

uint64_t TmpFSNode::size() {
	return file_size;
}

uint64_t TmpFSNode::entry_count() {
	return entries.size();
}

TmpFSNode* TmpFSNode::lookup(const char* name) {
	if (type != VFSNodeType::Directory) {
		Logger::log_error("TmpFS: Attempted to lookup directory \"%s\" in a File node.\n");
		return nullptr;
	}

	for (auto file : entries) {
		if (strcmp(file.name, name) == 0) {
			return file.node;
		}
	}

	return nullptr;
}

TmpFSNode* TmpFSNode::mkdir(const char* name) {
	if (type != VFSNodeType::Directory) {
		Logger::log_error("TmpFS: Attempted to create directory \"%s\" in a File node.\n");
		return nullptr;
	}

	for (auto file : entries) {
		if (strcmp(file.name, name) == 0) {
			if (file.node->type == VFSNodeType::Directory) {
				Logger::log_warning("TmpFS: The directory \"%s\" already exists.\n");
				return file.node;
			} else {
				Logger::log_error("TmpFS: The node \"%s\" already exists and is not a Directory.\n");
				return nullptr;
			}
		}
	}

	TmpFSNode* directory = new TmpFSNode();
	directory->type = VFSNodeType::Directory;
	directory->fs = fs;

	entries.push_back({ .name = strdup(name), .node = directory });

	return directory;
}

TmpFSNode* TmpFSNode::create(const char* name) {
	if (type != VFSNodeType::Directory) {
		Logger::log_error("TmpFS: Attempted to create file \"%s\" in a File node.\n");
		return nullptr;
	}

	for (auto file : entries) {
		if (strcmp(file.name, name) == 0) {
			if (file.node->type == VFSNodeType::File) {
				Logger::log_warning("TmpFS: The file \"%s\" already exists.\n");
				return file.node;
			} else {
				Logger::log_error("TmpFS: The node \"%s\" already exists and is not a File.\n");
				return nullptr;
			}
		}
	}

	TmpFSNode* file = new TmpFSNode();
	file->type = VFSNodeType::File;
	file->fs = fs;

	file->file_size = 4096;
	file->data = new uint8_t[4096];

	entries.push_back({ .name = strdup(name), .node = file });

	return file;
}

bool TmpFSNode::readdir(uint64_t index, DirectoryEntry* entry) {
	if (type != VFSNodeType::Directory) {
		Logger::log_error("TmpFS: Attempted to readdir a non-Directory node.\n");
		return false;
	}

	if (index > entries.size()) {
		return false;
	}

	TmpFSDirectoryEntry e = entries[index];

	size_t name_length = strlen(e.name);
	if (name_length > sizeof(entry->name)) {
		name_length = sizeof(entry->name);
	}

	memcpy(entry->name, e.name, name_length);
	entry->name[name_length] = '\0';

	return true;
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
