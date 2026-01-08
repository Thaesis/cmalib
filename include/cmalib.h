#if defined(_MSC_VER) || defined(__clang__) || defined(__GNUC__)
#   pragma once
#endif

#ifndef CMA_H_INCLUDE
#define CMA_H_INCLUDE

#include <cstddef>
#include <cstdint>
#include <bit>
#include <concepts>
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

    namespace impl {

        constexpr bool is_pow_2(std::unsigned_integral auto x) noexcept {
            return std::has_single_bit(x);
        }

        /**
         * @brief Rounds a pointer upward to the next address of a given alignment
         * @param p Raw byte pointer to round up.
         * @param alignment The alignment multiple to round up toward.
         * @returns The smallest address >= @c p that is a multiple of @c alignment.
         */
        constexpr std::byte* align_up(std::byte* p, std::size_t alignment) noexcept {
            const auto addr {reinterpret_cast<std::uintptr_t>(p)};

            /*
                This is a bit-trick in essense in order to achieve the proper rounding.

                1. We push the address forward such that truncation will round up.
                2. Apply a mask to clear the lower bits that (may) violate alignment.
            */
            const auto aligned {(addr + (alignment - 1)) & ~(alignment - 1)};
            return reinterpret_cast<std::byte*>(aligned);
        }

        /**
         * @brief Adds two elements with overflow detection bounded by the first element.
         */
        constexpr bool overflow_addition(std::size_t a, std::size_t b, std::size_t& res) noexcept {
            // I think this can be done with compiler directives? For now this is fine...
            res = a + b;
            return res < a;
        }

    } // namespace impl

    /**
     * @brief Smallest element of contigious raw-storage used by any given arena object.
     * 
     * A block should represent raw memory aligned for @c max_align_t.
     * 
     * @note I believe that this could be implemented without the use of linked-list architecture, in order to prioritize
     *       speed and further increase the contigious nature of the memory blocks.
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
         * %
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

        /**
         * @brief Specialized dtor of the @c block object, intended to free all reserved memory at once.
         */
        ~block() {
            ::operator delete(data);
            data = nullptr;
            cur = nullptr;
            end = nullptr;
            capacity = 0;
        }

    };

    /**
     * @brief Main memory handling class, utilizing linked memory blocks.
     */
    class arena {
    public:

        /**
         * @brief Marker used to roll-back some of the memory in the arena.
         */
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

        ~arena() { free_all(); }

        void* allocate_bytes(std::size_t bytes, std::size_t alignment = alignof(std::max_align_t)) {
            if(bytes == 0) { return nullptr; }

            // If function recieves some over-aligned alignment, we correct to max_align_t
            if(!impl::is_pow_2(alignment)) {
                alignment = alignof(std::max_align_t);
            }

            // Attempt current block
            if(void* p {try_alloc_at_active(bytes, alignment)}) { return p; }

            // Otherwise a new block is needed...
            std::size_t need {0};
            if(impl::overflow_addition(bytes, alignment, need)) { throw std::bad_alloc{}; }

            // Resizing policy dictates that we double our current capacity.
            std::size_t new_cap {std::max(_active->capacity * 2, need)};
            new_cap = std::max(new_cap, _block_size);

            auto* b {new block(new_cap)};
            b->next = nullptr;
            _active->next = b;
            _active = b;

            if(void* p {try_alloc_at_active(bytes, alignment)}) { return p; }

            throw std::bad_alloc{}; //This should be virtually impossible...
        }

    private:
        std::size_t _block_size {0};
        block* _head    {nullptr};
        block* _active  {nullptr};

        void free_all() noexcept {
            block* curr {_head};

            while(curr) {
                block* next {curr->next};
                delete curr;
                curr = next;
            }

            _head = nullptr;
            _active = nullptr;
        }

        void* try_alloc_at_active(std::size_t bytes, std::size_t alignment) noexcept {
            std::byte* aligned {impl::align_up(_active->cur, alignment)};

            if(static_cast<std::size_t>(_active->end - aligned) < bytes) { return nullptr; }

            _active->cur = aligned + bytes;
            return aligned;
        }
    };

}

#endif