#ifndef GLT_LOAD_MODELS_H
#define GLT_LOAD_MODELS_H

#include <unordered_map>
#include <string>
#include <vector>
#include <tinyobjloader/tiny_obj_loader.h>
#include "buffer_allocator.h"

namespace glt{
/*
 * Stores information about the offsets for some loaded model
 * index_offset: offset in number of indices to the indices for this model
 * indices: number of indices for the model
 * vert_offset: offset in number of vertices in the vert_buf to reach this
 * 				model's vertex data
 */
struct ModelInfo {
	size_t index_offset, indices, vert_offset;
	ModelInfo(size_t index_offset = 0, size_t indices = 0, size_t vert_offset = 0);
};
/*
 * Load all objects contained in the list of obj files using the buffer allocator
 * to allocate sub buffers `vert_buf` and `elem_buf` to store all the model information
 * elements will be stored as GLuints
 * vertex attribs are stored as interleaved vec3s in the order: pos, normal, texcoord
 * If a model doesn't have texcoords the texcoords will just be junk values
 * returns true if all models loaded successfully, false if not
 */
bool load_models(const std::vector<std::string> &model_files, SubBuffer &vert_buf,
		SubBuffer &elem_buf, BufferAllocator &allocator,
		std::unordered_map<std::string, ModelInfo> &elem_offsets);
}

#endif

