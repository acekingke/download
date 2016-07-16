#ifndef MEM_ALLOC_H
#define MEM_ALLOC_H

#include <cstddef>
#include <vector>
#include <assert.h>
#include <stdint.h>

class mem_allocter {
public:
	mem_allocter();
	~mem_allocter();

	// Return a pointer to a newly allocated memory block of "bytes" bytes.
	char* allocate(size_t bytes);

	// Allocate memory with the normal alignment guarantees provided by malloc
	char* allocate_aligned(size_t bytes);

	// Returns an estimate of the total memory usage of data allocated
	// by the mem_allocter (including space allocated but not yet used for user
	// allocations).
	size_t memory_usage() const {
		return blocks_memory_ + blocks_.capacity() * sizeof(char*);
	}

private:
	char* allocate_fallback(size_t bytes);
	char* allocate_newblock(size_t block_bytes);

	// Allocation state
	char* alloc_ptr_;
	size_t alloc_bytes_remaining_;

	// Array of new[] allocated memory blocks
	std::vector<char*> blocks_;

	// Bytes of memory in blocks allocated so far
	size_t blocks_memory_;

	// No copying allowed
	mem_allocter(const mem_allocter&);
	void operator=(const mem_allocter&);
};

inline char* mem_allocter::allocate(size_t bytes) {
	// The semantics of what to return are a bit messy if we allow
	// 0-byte allocations, so we disallow them here (we don't need
	// them for our internal use).
	assert(bytes > 0);
	if (bytes <= alloc_bytes_remaining_) {
		char* result = alloc_ptr_;
		alloc_ptr_ += bytes;
		alloc_bytes_remaining_ -= bytes;
		return result;
	}
	return allocate_fallback(bytes);
}


#else
#endif  