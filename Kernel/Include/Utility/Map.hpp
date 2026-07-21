#pragma once

#include <Utility/Vector.hpp>

template<typename K, typename V>
class Map {
public:
	struct Entry {
		K key;
		V value;
	};

	Map() = default;

	~Map() = default;

	Map(Map&& other)
		: entries(static_cast<Vector<Entry>&&>(other.entries)) {}

	Map& operator=(Map&& other) {
		if (this != &other) {
			entries = static_cast<Vector<Entry>&&>(other.entries);
		}

		return *this;
	}

	Map(const Map& other)
		: entries(other.entries) {}

	Map& operator=(const Map& other) {
		if (this != &other) {
			entries = other.entries;
		}

		return *this;
	}

	void insert(const K& key, const V& value) {
		for (size_t i = 0; i < entries.size(); i++) {
			if (entries[i].key == key) {
				entries[i].value = value;
				return;
			}
		}

		entries.push_back({ key, value });
	}

	bool contains(const K& key) const {
		for (size_t i = 0; i < entries.size(); i++) {
			if (entries[i].key == key)
				return true;
		}

		return false;
	}

	V* get(const K& key) {
		for (size_t i = 0; i < entries.size(); i++) {
			if (entries[i].key == key)
				return &entries[i].value;
		}

		return nullptr;
	}

	const V* get(const K& key) const {
		for (size_t i = 0; i < entries.size(); i++) {
			if (entries[i].key == key)
				return &entries[i].value;
		}

		return nullptr;
	}

	void remove(const K& key) {
		for (size_t i = 0; i < entries.size(); i++) {
			if (entries[i].key == key) {
				entries.erase(i);
				return;
			}
		}
	}

	V& operator[](const K& key) {
		V* value = get(key);

		if (value)
			return *value;

		entries.push_back({ key, V {} });

		return entries[entries.size() - 1].value;
	}

	size_t size() const {
		return entries.size();
	}

	bool empty() const {
		return entries.empty();
	}

	void clear() {
		entries.clear();
	}

private:
	Vector<Entry> entries;
};
