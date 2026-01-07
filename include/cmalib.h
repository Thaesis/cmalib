#if defined(_MSC_VER) || defined(__clang__) || defined(__GNUC__)
#   pragma once
#endif

#ifndef CMA_H_INCLUDE
#define CMA_H_INCLUDE

#include <cstddef>
#include <new>
#include <algorithm>

/**
 * Since this is meant to be exploratory in nature, I will include guarantees and important elements to defining arena allocators.
 * 
 * ...Normally-aligned & Over-alinged objects...
 * 
 * Normally-aligned: If an object has an alignment <= alignof(std::max_align_t)
 * Over-aligned: If an object has an alignment > alignof(std::max_align_t)
 * 
 * alignof(T) = MAX(alignof(all non-static data members and base classes))
 * Essentially, since memory is retrieved in "blocks", we have to ensure that our alignment of 
 * bytes is based on the largest alignment of any given type in an object. Thus, we are using
 * ::operator new() always gives memory aligned well-enough for any standard C++ object.
 * This is important for our use of memory arenas alongside Standard-Library data-structures.
 * 
 */ 

namespace cma {

/**
 * 
 */
struct block {

    /// @brief Start of raw storage for this block (size = capacity)
    std::byte* data {nullptr};

    /// @brief Next free byte within the block; [data, end].
    std::byte* cur  {nullptr};

    /// @brief One past the last byte of this block's storage.
    std::byte* end  {nullptr};

    /// @brief Next block in the linked-list.
    block* next     {nullptr};

    /// @brief Size of the block's storage in bytes.
    std::size_t capacity {0};

    /**
     * @brief Standard constructor for a @c block object.
     * 
     * The block is initialized with data aligned to @c max_align_t, ensuring that any normally-aligned
     * object can be initialized or stored in the block.
     * 
     * @param bytes The size of the block to allocate in bytes.
     */
    explicit block(std::size_t bytes)
        : data{static_cast<std::byte*>(::operator new(bytes))}  // ::operator new returns memory that is aligned for max_align_t
        , cur{data}
        , end{data + bytes}
        , capacity{bytes}
    {}

    /**
     * @brief Removal of copy-constructor due to memory safety requirements.
     */
    block(const block&) = delete;

    /**
     * @brief Removal of the assignment operator due to memory safety requirements.
     */
    block& operator=(const block&) = delete;

    ~block() {
        ::operator delete(data);
        data = nullptr;
        cur = nullptr;
        end = nullptr;
        capacity = 0;
    }

};

class arena {
public:

    struct marker {
        block* block    {nullptr};
        std::byte* cur  {nullptr};
    };

    explicit arena(std::size_t initial_block_size = 64 * 1024)
        : _block_size{std::max<std::size_t>(initial_block_size, 1024)}
        , _head{new block(_block_size)}
        , _active{new block(_block_size)}
    {}

    arena(const arena&) = delete;

    arena& operator=(const arena&) = delete;

private:
    std::size_t _block_size {0};
    block* _head    {nullptr};
    block* _active  {nullptr};
};

}

#endif