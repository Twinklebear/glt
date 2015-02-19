#include <iostream>
#include <algorithm>
#include <numeric>
#include "glt/load_models.h"

glt::ModelInfo::ModelInfo(size_t index_offset, size_t indices, size_t vert_offset)
	: index_offset(index_offset), indices(indices), vert_offset(vert_offset)
{}
bool glt::load_models(const std::vector<std::string> &model_files, SubBuffer &vert_buf,
		SubBuffer &elem_buf, BufferAllocator &allocator,
		std::unordered_map<std::string, ModelInfo> &elem_offsets)
{
	using namespace glt;
	std::vector<tinyobj::shape_t> loaded_models;
	for (const auto &file : model_files){
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string err = tinyobj::LoadObj(shapes, materials, file.c_str());
		if (!err.empty()){
			std::cout << "Failed to load model " << file << " error: " << err << std::endl;
			return false;
		}
		std::copy(shapes.begin(), shapes.end(), std::back_inserter(loaded_models));
	}

	size_t total_elems = std::accumulate(loaded_models.begin(), loaded_models.end(), 0,
			[](const size_t &cur, const tinyobj::shape_t &s){
				return cur + s.mesh.indices.size();
			});
	elem_buf = allocator.alloc(total_elems * sizeof(GLuint), sizeof(GLuint));
	{
		unsigned int *elems = static_cast<unsigned int*>(elem_buf.map(GL_ELEMENT_ARRAY_BUFFER,
					GL_MAP_INVALIDATE_RANGE_BIT | GL_MAP_WRITE_BIT));
		// Track our offset in the element count
		size_t prev_offset = 0;
		for (const auto &s : loaded_models){
			elem_offsets[s.name] = ModelInfo{prev_offset, s.mesh.indices.size()};
			std::copy(s.mesh.indices.begin(), s.mesh.indices.end(), elems + prev_offset);
			prev_offset += s.mesh.indices.size();
		}
		elem_buf.unmap(GL_ELEMENT_ARRAY_BUFFER);
	}

	// We store 6 floats per element at the moment
	// Format is vec3 (pos), vec3 (normal), vec2 (texcoord)
	vert_buf = allocator.alloc(total_elems * 8 * sizeof(float));
	{
		float *verts = static_cast<float*>(vert_buf.map(GL_ARRAY_BUFFER,
					GL_MAP_INVALIDATE_RANGE_BIT | GL_MAP_WRITE_BIT));
		// Track our offset in the vertex buffer
		size_t i = 0;
		for (const auto &s : loaded_models){
			elem_offsets[s.name].vert_offset = i / 8;
			for (auto p = s.mesh.positions.begin(), n = s.mesh.normals.begin(), t = s.mesh.texcoords.begin();
					p != s.mesh.positions.end() && n != s.mesh.normals.end();
					i += 8)
			{
				for (int k = 0; k < 3; ++k, ++p){
					verts[i + k] = *p;
				}
				for (int k = 3; k < 6; ++k, ++n){
					verts[i + k] = *n;
				}
				// Some models may not have/need texcoords
				if (t != s.mesh.texcoords.end()){
					for (int k = 6; k < 8; ++k, ++t){
						verts[i + k] = *t;
					}
				}
			}
		}
		vert_buf.unmap(GL_ARRAY_BUFFER);
	}
	return true;
}

