#include "mem_alloc.h"

static const int kBlockSize = 4096;

mem_allocter::mem_allocter() {
	blocks_memory_ = 0;
	alloc_ptr_ = NULL;  // First allocation will allocate a block
	alloc_bytes_remaining_ = 0;
}

mem_allocter::~mem_allocter() {
	for (size_t i = 0; i < blocks_.size(); i++) {
		delete[] blocks_[i];
	}
}

char* mem_allocter::allocate_fallback(size_t bytes) {
	if (bytes > kBlockSize / 4) {
		// Object is more than a quarter of our block size.  Allocate it separately
		// to avoid wasting too much space in leftover bytes.
		char* result = allocate_newblock(bytes);
		return result;
	}

	// We waste the remaining space in the current block.
	alloc_ptr_ = allocate_newblock(kBlockSize);
	alloc_bytes_remaining_ = kBlockSize;

	char* result = alloc_ptr_;
	alloc_ptr_ += bytes;
	alloc_bytes_remaining_ -= bytes;
	return result;
}

char* mem_allocter::allocate_aligned(size_t bytes) {
	const int align = sizeof(void*);    // We'll align to pointer size
	assert((align & (align-1)) == 0);   // Pointer size should be a power of 2
	size_t current_mod = reinterpret_cast<uintptr_t>(alloc_ptr_) & (align-1);
	size_t slop = (current_mod == 0 ? 0 : align - current_mod);
	size_t needed = bytes + slop;
	char* result;
	if (needed <= alloc_bytes_remaining_) {
		result = alloc_ptr_ + slop;
		alloc_ptr_ += needed;
		alloc_bytes_remaining_ -= needed;
	} else {
		// AllocateFallback always returned aligned memory
		result = allocate_fallback(bytes);
	}
	assert((reinterpret_cast<uintptr_t>(result) & (align-1)) == 0);
	return result;
}

char* mem_allocter::allocate_newblock(size_t block_bytes) {
	char* result = new char[block_bytes];
	blocks_memory_ += block_bytes;
	blocks_.push_back(result);
	return result;
}