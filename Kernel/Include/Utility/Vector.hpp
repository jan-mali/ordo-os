#pragma once

#include <cstddef>

template<typename T>
class Vector {
public:
	Vector()
		: data(nullptr)
		, size_(0)
		, capacity_(0) {}

	~Vector() {
		delete[] data;
	}

	// Copy disabled for now (kernel simplicity)
	Vector(const Vector&) = delete;
	Vector& operator=(const Vector&) = delete;

	// Move constructor
	Vector(Vector&& other)
		: data(other.data)
		, size_(other.size_)
		, capacity_(other.capacity_) {
		other.data = nullptr;
		other.size_ = 0;
		other.capacity_ = 0;
	}

	// Move assignment operator
	Vector& operator=(Vector&& other) {
		if (this != &other) {
			delete[] data;

			data = other.data;
			size_ = other.size_;
			capacity_ = other.capacity_;

			other.data = nullptr;
			other.size_ = 0;
			other.capacity_ = 0;
		}

		return *this;
	}

	void push_back(const T& value) {
		if (size_ >= capacity_)
			grow();

		data[size_++] = value;
	}

	void pop_back() {
		if (size_ > 0)
			size_--;
	}

	T remove_back() {
		if (size_ == 0)
			return T {};

		T value = static_cast<T&&>(data[size_ - 1]);
		size_--;

		return value;
	}

	T remove_front() {
		if (size_ == 0)
			return T {};

		T value = static_cast<T&&>(data[0]);

		for (size_t i = 1; i < size_; i++) {
			data[i - 1] = static_cast<T&&>(data[i]);
		}

		size_--;

		return value;
	}

	T remove_index(size_t index) {
		if (index >= size_)
			return T {};

		T value = static_cast<T&&>(data[index]);

		for (size_t i = index + 1; i < size_; i++) {
			data[i - 1] = static_cast<T&&>(data[i]);
		}

		size_--;

		return value;
	}

	void erase(size_t index) {
		if (index >= size_)
			return;

		for (size_t i = index + 1; i < size_; i++) {
			data[i - 1] = static_cast<T&&>(data[i]);
		}

		size_--;
	}

	T& operator[](size_t index) {
		return data[index];
	}

	const T& operator[](size_t index) const {
		return data[index];
	}

	T* begin() {
		return data;
	}

	T* end() {
		return data + size_;
	}

	size_t size() const {
		return size_;
	}

	size_t capacity() const {
		return capacity_;
	}

	bool empty() const {
		return size_ == 0;
	}

	void clear() {
		size_ = 0;
	}

private:
	T* data;
	size_t size_;
	size_t capacity_;

	void grow() {
		size_t new_capacity = capacity_ == 0 ? 8 : capacity_ * 2;

		T* new_data = new T[new_capacity];

		for (size_t i = 0; i < size_; i++) {
			new_data[i] = static_cast<T&&>(data[i]);
		}

		delete[] data;

		data = new_data;
		capacity_ = new_capacity;
	}
};
