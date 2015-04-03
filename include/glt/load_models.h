#ifndef GLT_LOAD_MODELS_H
#define GLT_LOAD_MODELS_H

#include <unordered_map>
#include <ostream>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <tiny_obj_loader.h>
#include "buffer_allocator.h"
#include "load_texture.h"

namespace glt {
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
 * Stores information about the offsets for some loaded model along with it's material id
 * index_offset: offset in number of indices to the indices for this model
 * indices: number of indices for the model
 * vert_offset: offset in number of vertices in the vert_buf to reach this
 * 				model's vertex data
 */
struct ModelMatInfo {
	size_t index_offset, indices, vert_offset, mat_id;
	ModelMatInfo(size_t index_offset = 1, size_t indices = 0, size_t vert_offset = 0,
			size_t mat_id = 0);
};
/*
 * Information about a model's material stored in the material buffer
 * Matches the struct defined in global.glsl, struct is 80 bytes so it will
 * align properly in a std140 array and host-side array easily
 * Texture map info will be -1 if no texture is used
 */
struct Material {
	glm::vec4 ka;
	glm::vec4 kd;
	glm::vec4 ks;
	// Stores { ambient_texture_index, ambient_texture_layer, diff_texture_index, diff_texture_layer }
	glm::ivec4 map_ka_kd;
	glm::ivec4 map_ks_n;
	glm::ivec4 map_mask;

	Material(glm::vec4 ka, glm::vec4 kd, glm::vec4 ks, glm::ivec4 map_ka_kd, glm::ivec4 map_ks_n,
			glm::ivec4 map_mask);
};
/*
 * Load all objects contained in the list of obj files using the buffer allocator
 * to allocate sub buffers `vert_buf` and `elem_buf` to store all the model information
 * elements will be stored as GLuints
 * vertex attribs are stored as interleaved vecs in the order:
 * 	vec3 pos, vec3 normal, vec2 texcoord
 * If a model doesn't have texcoords the texcoords will just be junk values
 * returns true if all models loaded successfully, false if not
 */
bool load_models(const std::vector<std::string> &model_files, SubBuffer &vert_buf,
		SubBuffer &elem_buf, BufferAllocator &allocator,
		std::unordered_map<std::string, ModelInfo> &elem_offsets);
/*
 * Load the model specified along with its materials. Fills out the vert and
 * elem buffers as before but also loads textures and material info (int mat_buf).
 * The material ids are returned per object as well in the ModelMatInfo map
 */
bool load_model_with_mats(const std::string &model_file, BufferAllocator &allocator,
		SubBuffer &vert_buf, SubBuffer &elem_buf, SubBuffer &mat_buf,
		OBJTextures &obj_textures, std::unordered_map<std::string, ModelMatInfo> &model_info);
}
std::ostream& operator<<(std::ostream &os, const glt::ModelInfo &m);
std::ostream& operator<<(std::ostream &os, const glt::Material &m);

#endif

