#include <vector>
#include <map>
#include <algorithm>
#include <cassert>
#include <iostream>
#include <iterator>
#include "gl_core_4_5.h"
#include "glt/buffer_allocator.h"

using namespace glt;

glt::SubBuffer::SubBuffer(size_t offset, size_t size, GLuint buf)
	: offset(offset), size(size), buffer(buf)
{}
void* glt::SubBuffer::map(GLenum target, GLenum access){
	assert(size != 0);
	// TODO: Direct state access?
	glBindBuffer(target, buffer);
	return glMapBufferRange(target, offset, size, access);
}
void glt::SubBuffer::unmap(GLenum target){
	glBindBuffer(target, buffer);
	glUnmapBuffer(target);
}

glt::Buffer::Buffer(size_t size) : size(size){
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	if (ogl_IsVersionGEQ(4, 4)){
		glBufferStorage(GL_ARRAY_BUFFER, size, NULL, GL_MAP_WRITE_BIT | GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT);
	}
	else {
		// Nvidia seems to put buffer storage created buffers with write | read as dynamic draw so we'll
		// use that as our fallback usage as well
		glBufferData(GL_ARRAY_BUFFER, size, NULL, GL_DYNAMIC_DRAW);
	}
	freeb.insert(std::make_pair(0, Block { 0, size }));
}
glt::Buffer::Buffer(Buffer &&b) : size(b.size), buffer(b.buffer), freeb(std::move(b.freeb)), used(std::move(b.used)){
	b.size = 0;
	b.buffer = 0;
}
glt::Buffer::~Buffer(){
	if (size != 0){
		glDeleteBuffers(1, &buffer);
	}
}
bool glt::Buffer::contains(SubBuffer &b) const {
	return buffer == b.buffer;
}
bool glt::Buffer::alloc(size_t sz, SubBuffer &buf, size_t align){
	if (freeb.empty()){
		return false;
	}
	for (auto b = freeb.begin(); b != freeb.end(); ++b){
		// Account for alignment requirements of allocation requests
		size_t align_offset = b->second.offset % align == 0 ? 0
			: align - b->second.offset % align;
		if (sz + align_offset <= b->second.size){
			// The actual offset of the block we're allocating, accounting for alignment
			size_t offset = b->second.offset + align_offset;
			buf = SubBuffer(offset, sz, buffer);
			size_t rem = b->second.size - sz;
			Block block = b->second;
			// We have to re-insert since the block's offset will change
			// even if we still have some free space left
			freeb.erase(b);
			if (rem == 0){
				used.insert(std::make_pair(block.offset, block));
			}
			else {
				used.insert(std::make_pair(offset, Block { offset, sz }));
				freeb.insert(std::make_pair(block.offset + sz + align_offset,
							Block { block.offset + sz + align_offset, block.size - sz - align_offset }));
				// If we left some in front of the block due to alignment requirements insert that block too
				if (align_offset != 0){
					freeb.insert(std::make_pair(block.offset, Block { block.offset, align_offset }));
				}
			}
			return true;
		}
	}
	return false;
}
bool glt::Buffer::realloc(SubBuffer &b, size_t new_sz){
	if (freeb.empty() || !contains(b)){
		return false;
	}
	auto u = used.find(b.offset);
	// See if the block after the one used by buf is available
	auto fnd = freeb.find(b.offset + b.size);
	if (fnd != freeb.end() && new_sz <= b.size + fnd->second.size){
		const Block block = fnd->second;
		freeb.erase(fnd);
		size_t amt = new_sz - b.size;
		u->second.size += amt;
		if (amt != block.size){
			freeb.insert(std::make_pair(block.offset + amt, Block { block.offset + amt, block.size - amt }));
		}
		b.size = new_sz;
		return true;
	}
	// We can't expand the used block so try to find a free block in the buffer to copy over to
	SubBuffer c;
	if (alloc(new_sz, c)){
		// Enqueue device-side copy to move the data over to the new sub-buffer
		// TODO: DSA?
		glBindBuffer(GL_COPY_READ_BUFFER, buffer);
		glBindBuffer(GL_COPY_WRITE_BUFFER, buffer);
		glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, b.offset, c.offset, b.size);
		free(b);
		b = c;
		return true;
	}
	return false;
}
void glt::Buffer::free(SubBuffer &buf){
	assert(contains(buf) && !used.empty());
	auto fnd = used.find(buf.offset);
	assert(fnd != used.end());
	Block rel = fnd->second;
	used.erase(fnd);
	freeb.insert(std::make_pair(rel.offset, rel));
	// Step through the free blocks and merge neighbors
	for (auto it = freeb.begin(); it != std::prev(freeb.end());){
		auto next = std::next(it);
		// Consecutive free blocks to be merged
		if (it->second.offset + it->second.size == next->second.offset){
			it->second.size += next->second.size;
			freeb.erase(next);
		}
		else {
			++it;
		}
	}
}

glt::BufferAllocator::BufferAllocator(size_t capacity) : capacity(capacity){
	buffers.emplace_back(capacity);
}
SubBuffer glt::BufferAllocator::alloc(size_t sz, size_t align){
	SubBuffer buf;
	for (auto &b : buffers){
		if (b.alloc(sz, buf, align)){
			return buf;
		}
	}
	size_t new_size = std::max(capacity, sz);
	buffers.emplace_back(new_size);
	if (buffers.back().alloc(sz, buf, align)){
		return buf;
	}
	std::cout << "Failed to allocate enough room still?\n";
	assert(false);
	return SubBuffer{};
}
void glt::BufferAllocator::realloc(SubBuffer &b, size_t new_sz){
	// First try to realloc within the buffer that this buffer was allocated from
	auto it = std::find_if(buffers.begin(), buffers.end(), [&](const Buffer &buf){ return buf.contains(b); });
	if (it == buffers.end()){
		std::cout << "Error: attempt to re-alloc buffer not in this allocator\n";
		return;
	}
	// If we need to push on a new buffer we may invalidate our iterator so grab the index instead
	size_t parent = std::distance(buffers.begin(), it);
	// If the parent can't meet the realloc we need to find a new home and copy the data over
	if (!buffers[parent].realloc(b, new_sz)){
		SubBuffer new_buf = alloc(new_sz);
		// Enqueue device-side copy to move the data over to the new sub-buffer
		// TODO: DSA?
		glBindBuffer(GL_COPY_READ_BUFFER, b.buffer);
		glBindBuffer(GL_COPY_WRITE_BUFFER, new_buf.buffer);
		glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, b.offset, new_buf.offset, b.size);
		buffers[parent].free(b);
		b = new_buf;
	}

}
void glt::BufferAllocator::free(SubBuffer &buf){
	for (auto &b : buffers){
		if (b.contains(buf)){
			b.free(buf);
			buf.size = 0;
			return;
		}
	}
	std::cout << "Warning: Found no buffer containing SubBuffer to free from\n";
}

std::ostream& operator<<(std::ostream &os, const SubBuffer &b){
	os << "SubBuffer { offset: " << b.offset << ", size: " << b.size
		<< ", buffer: " << b.buffer << " }";
	return os;
}
std::ostream& operator<<(std::ostream &os, const Block &b){
	os << "Block { offset: " << b.offset << ", size: " << b.size << " }";
	return os;
}
std::ostream& operator<<(std::ostream &os, const Buffer &b){
	os << "Buffer { size: " << b.size << ", buffer: " << b.buffer
		<< "\n\tfree blocks:\n";
	for (const auto &f : b.freeb){
		os << "\t\t" << f.second << "\n";
	}
	os << "\tused blocks:\n";
	for (const auto &u : b.used){
		os << "\t\t" << u.second << "\n";
	}
	os << "\t}";
	return os;
}
std::ostream& operator<<(std::ostream &os, const BufferAllocator &b){
	os << "BufferAllocator { buffers:\n";
	for (const auto &buf : b.buffers){
		os << "\t" << buf << "\n";
	}
	os << "}";
	return os;
}

