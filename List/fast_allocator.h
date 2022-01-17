#pragma once

#include <new>
#include <vector>
#include <cassert>
#include <iostream>
#include <functional>

namespace detail {
// this is singleton
template <size_t chunkSize>
class FixedAllocator {
private:
    std::vector<void*> data_;
    std::vector<void*> unused_;
    size_t lastIdx_ = 0;

    void resizeIfFull() {
        if (lastIdx_ == 0) {
            lastIdx_ = chunkSize * (1 << data_.size());
            void* newPtr = ::operator new(lastIdx_);
            data_.push_back(newPtr);
        }
    }

    FixedAllocator() = default;

    ~FixedAllocator() {
        for (auto* i : data_) {
            ::operator delete(i);
        }
    }

public:
    static FixedAllocator<chunkSize>& instance() {
        static FixedAllocator<chunkSize> singleton;
        return singleton;
    }

    void* allocate() {
        if (!unused_.empty()) {
            auto ans = unused_.back();
            unused_.pop_back();
            return ans;
        }
        resizeIfFull();
        char* rep = reinterpret_cast<char*>(data_.back());
        lastIdx_ -= chunkSize;
        auto ans = reinterpret_cast<void*>(rep + lastIdx_);
        return ans;
    }

    void deallocate(void* ptr) {
        unused_.push_back(ptr);
    }
};

template <typename T, size_t chunkSize = 8>
struct allocateChunk {
    static T* allocate(size_t n) {
        if constexpr (chunkSize > 64) {
            return static_cast<T*>(::operator new(n * sizeof(T)));
        } else {
            if (n * sizeof(T) <= chunkSize) {
                return static_cast<T*>(FixedAllocator<chunkSize>::instance().allocate());
            }
            return allocateChunk<T, chunkSize + 8>::allocate(n);
        }
    }
};

template <typename T, size_t chunkSize = 8>
struct deallocateChunk {
    static void deallocate(void* ptr, size_t n) {
        if constexpr (chunkSize > 64) {
            return ::operator delete(ptr);
        } else {
            if (n * sizeof(T) <= chunkSize) {
                return FixedAllocator<chunkSize>::instance().deallocate(ptr);
            }
            return deallocateChunk<T, chunkSize + 8>::deallocate(ptr, n);
        }
    }
};
}

template <typename T>
class FastAllocator {
public:
    using value_type = T;
    using size_type = size_t;
    using difference_type = ptrdiff_t;

    FastAllocator() = default;
    FastAllocator(const FastAllocator<T>&) = default;
    ~FastAllocator() = default;
    template <typename U>
    FastAllocator(const FastAllocator<U>&): FastAllocator() {}

    T* allocate(size_t n) {
        return detail::allocateChunk<T>::allocate(n);
    }

    void deallocate(T* ptr, size_t n) {
        return detail::deallocateChunk<T>::deallocate(ptr, n);
    }
};

template <typename T, typename U>
bool operator==(const FastAllocator<T>&, const FastAllocator<U>&) {
    return std::is_same_v<T, U>;
}

template <typename T, typename U>
bool operator!=(const FastAllocator<T>& a, const FastAllocator<U>& b) {
    return !(a == b);
}
