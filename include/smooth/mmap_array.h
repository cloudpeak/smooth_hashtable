#ifndef HASHMAP_MMAP_ARRAY_H
#define HASHMAP_MMAP_ARRAY_H

#include <stdexcept>
#include <cstring>

#ifdef _WIN32
#include <windows.h>

// Platform-specific functions for Windows
void* platform_mmap(void* addr, size_t len, int fd, off_t offset) {
    void * data = VirtualAlloc(nullptr, len, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if(data == nullptr) {
        return reinterpret_cast<void*>(-1);
    }
    return data;
}

int platform_munmap(void* addr, size_t len) {
    // for VirtualFree
    // If the function succeeds, the return value is nonzero.
    // If the function fails, the return value is 0 (zero)
    BOOL ret = VirtualFree(addr, 0, MEM_RELEASE);
    if(ret) {
        return 0;
    }
    return -1;
}

#else // Linux

#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#ifndef MAP_ANONYMOUS
# define MAP_ANONYMOUS MAP_ANON
#endif

// Platform-specific functions for Linux
void* platform_mmap(void* addr, size_t len,  int fd, off_t offset) {
  return mmap(addr, len, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, fd, offset);
}

int platform_munmap(void* addr, size_t len) {
    // Upon successful completion, munmap() shall return 0; otherwise, it shall return -1 and set errno to indicate the error.
  return munmap(addr, len);
}

#endif

const size_t k_threshold_for_mmap = 4096;

template <typename T>
class mmap_array {
public:
    mmap_array() : size_(0), data_(nullptr) {}

    explicit mmap_array(size_t size) : size_(size) {
      // Allocate memory using appropriate method
      size_t size_in_bytes = size_ * sizeof(T);
      if (size_in_bytes < k_threshold_for_mmap) {
        // Allocate using new if size is larger than 4k
        char* ptr = new char[size_in_bytes];
        std::memset(ptr, 0, size_in_bytes);
        data_ = reinterpret_cast<T*>(ptr);
      } else {
        // Allocate using platform_mmap if size is smaller than 4k
        data_ = static_cast<T*>(platform_mmap(nullptr, size_in_bytes, -1, 0));
        if (data_ == reinterpret_cast<void*>(-1)) {
          throw std::runtime_error("Error mapping memory");
        }
      }
    }

    ~mmap_array() {
      clear();
    }

    void clear() {
      if (data_ != nullptr) {
        size_t  size_in_bytes = size_ * sizeof(T);
        if (size_in_bytes < k_threshold_for_mmap) {
            char* ptr = reinterpret_cast<char*>(data_);
            delete [] ptr;
        } else {
          if (platform_munmap(data_, size_in_bytes) == -1) {
            assert(0);
          }
        }
        data_ = nullptr;
      }
      size_ = 0;
    }

    mmap_array(const mmap_array&) = delete;
    mmap_array& operator=(const mmap_array&) = delete;

    mmap_array(mmap_array&& other) noexcept {
      data_ = other.data_;
      size_ = other.size_;
      other.data_ = nullptr;
      other.size_ = 0;
    }

    void swap(mmap_array& other) {
      std::swap(data_, other.data_);
      std::swap(size_, other.size_);
    }

    mmap_array& operator=(mmap_array&& other) noexcept {
      if (this != &other) {
        clear();
        data_ = other.data_;
        size_ = other.size_;
        other.data_ = nullptr;
        other.size_ = 0;
      }
      return *this;
    }

    T* data() { return data_; }
    size_t size() const { return size_; }

    // Access element at a specific index (non-const)
    T& at(size_t index) {
      if (index >= size_) {
        throw std::out_of_range("Index out of bounds");
      }
      return data_[index];
    }

    // Access element at a specific index (const)
    const T& at(size_t index) const {
      if (index >= size_) {
        throw std::out_of_range("Index out of bounds");
      }
      return data_[index];
    }

    // Access element at a specific index using operator[] (non-const)
    T& operator[](size_t index) {
      return data_[index];
    }

    // Access element at a specific index using operator[] (const)
    const T& operator[](size_t index) const {
      return data_[index];
    }

private:

    int64_t i64_size() const { return static_cast<int64_t>(size_); }

    T* data_;
    size_t size_;
};


#endif //HASHMAP_MMAP_ARRAY_H