#ifndef GLT_BUFFER_ALLOCATOR_H
#define GLT_BUFFER_ALLOCATOR_H

#include <vector>
#include <map>
#include <iterator>
#include "gl_core_4_5.h"

namespace glt {
	class Buffer;
	class BufferAllocator;
}

std::ostream& operator<<(std::ostream &os, const glt::Buffer &b);
std::ostream& operator<<(std::ostream &os, const glt::BufferAllocator &b);

namespace glt {
// An allocated sub buffer within some large buffer
struct SubBuffer {
	size_t offset, size;
	GLuint buffer;

	SubBuffer(size_t offset = 0, size_t size = 0, GLuint buf = 0);
	// Map this sub buffer and return a pointer to the mapped range
	void* map(GLenum target, GLenum access);
	// Unamp this sub buffer
	void unmap(GLenum target);
};

// A block of memory in the buffer
struct Block {
	size_t offset, size;
};

// A large buffer that can hand out sub buffers to satisfy allocation requests
class Buffer {
	size_t size;
	GLuint buffer;
	std::map<size_t, Block> freeb, used;

	friend std::ostream& ::operator<<(std::ostream &os, const glt::Buffer &b);
public:
	// Allocate a buffer with some capacity
	Buffer(size_t size);
	Buffer(const Buffer&) = delete;
	Buffer& operator=(const Buffer&) = delete;
	Buffer(Buffer &&b);
	~Buffer();
	// Check if the sub buffer was allocated from this block's data store
	bool contains(SubBuffer &b) const;
	// Allocate a sub buffer with some capacity, returns
	// true if the buffer was able to satisfy the request
	bool alloc(size_t sz, SubBuffer &buf, size_t align = 1);
	// Reallocate a sub buffer to some new (larger) capacity, returns true if the
	// buffer was able to meet the request. The buffer be realloc'd should
	// be one allocated in this buffer
	bool realloc(SubBuffer &b, size_t new_sz);
	// Free the block used by the sub buffer and merge and neighboring free blocks
	void free(SubBuffer &buf);
};

// A buffer allocator that will use Buffers to meet allocation requests. If the allocator
// runs out of free space in its buffers it will allocate another to meet demand
class BufferAllocator {
	size_t capacity;
	std::vector<Buffer> buffers;

	friend std::ostream& ::operator<<(std::ostream &os, const glt::BufferAllocator &b);
public:
	// Create a buffer allocator which will allocate memory in chunks of `capacity`
	BufferAllocator(size_t capacity);
	// Allocate a sub buffer of some size within some free space in the allocator's buffers
	SubBuffer alloc(size_t sz, size_t align = 1);
	// Reallocate a sub buffer to some new (larger) capacity. If there's enough room after
	// the buffer in the parent it will simply be expanded otherwise the data
	// may be moved within the parent or to a new buffer in the allocator
	void realloc(SubBuffer &b, size_t new_sz);
	// Free the sub buffer so that the used space may be re-used
	void free(SubBuffer &buf);
};
}
std::ostream& operator<<(std::ostream &os, const glt::SubBuffer &b);
std::ostream& operator<<(std::ostream &os, const glt::Block &b);

#endif

