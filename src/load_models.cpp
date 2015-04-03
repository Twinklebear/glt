#include <iostream>
#include <set>
#include <algorithm>
#include <numeric>
#include <glm/ext.hpp>
#include "glt/util.h"
#include "glt/load_models.h"

glt::ModelInfo::ModelInfo(size_t index_offset, size_t indices, size_t vert_offset)
	: index_offset(index_offset), indices(indices), vert_offset(vert_offset)
{}

glt::ModelMatInfo::ModelMatInfo(size_t index_offset, size_t indices, size_t vert_offset, size_t mat_id)
	: index_offset(index_offset), indices(indices), vert_offset(vert_offset), mat_id(mat_id)
{}

glt::Material::Material(glm::vec4 ka, glm::vec4 kd, glm::vec4 ks, glm::ivec4 map_ka_kd,
		glm::ivec4 map_ks_n, glm::ivec4 map_mask)
	: ka(ka), kd(kd), ks(ks), map_ka_kd(map_ka_kd), map_ks_n(map_ks_n), map_mask(map_mask)
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
		std::string base_path;
		const auto base_path_end = file.rfind(PATH_SEP);
		if (base_path_end != std::string::npos){
			base_path = file.substr(0, base_path_end + 1);
		}
		std::string err = tinyobj::LoadObj(shapes, materials, file.c_str(), base_path.c_str());
		if (!err.empty()){
			std::cout << "Failed to load model " << file << " error: " << err << std::endl;
			return false;
		}
		std::cout << "loaded " << shapes.size() << " model(s) from " << file << ", name(s):\n";
		for (const auto &s : shapes){
			std::cout << "\t" << s.name;
			if (!materials.empty()){
				std::cout << ", uses material: " << materials[s.mesh.material_ids[0]].name << "\n";
			}
			else {
				std::cout << "\n";
			}
		}
		std::cout << "loaded " << materials.size() << " material(s):\n";
		for (const auto &m : materials){
			std::cout << "\t" << m.name << "\n";
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
					p != s.mesh.positions.end();
					i += 8)
			{
				for (int k = 0; k < 3; ++k, ++p){
					verts[i + k] = *p;
				}
				if (n != s.mesh.normals.end()){
					for (int k = 3; k < 6; ++k, ++n){
						verts[i + k] = *n;
					}
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
bool glt::load_model_with_mats(const std::string &model_file, BufferAllocator &allocator,
		SubBuffer &vert_buf, SubBuffer &elem_buf, SubBuffer &mat_buf,
		OBJTextures &obj_textures, std::unordered_map<std::string, ModelMatInfo> &model_info)
{
	using namespace glt;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string base_path;
	const auto base_path_end = model_file.rfind(PATH_SEP);
	if (base_path_end != std::string::npos){
		base_path = model_file.substr(0, base_path_end + 1);
	}
	std::string err = tinyobj::LoadObj(shapes, materials, model_file.c_str(), base_path.c_str());
	if (!err.empty()){
		std::cout << "Failed to load model " << model_file << " error: " << err << std::endl;
		return false;
	}
	std::cout << "loaded " << shapes.size() << " model(s) from " << model_file << ", name(s):\n";
	for (const auto &s : shapes){
		std::cout << "\t" << s.name;
		if (!materials.empty()){
			std::cout << ", uses material: " << materials[s.mesh.material_ids[0]].name << "\n";
		}
		else {
			std::cout << "\n";
		}
	}
	std::cout << "loaded " << materials.size() << " material(s):\n";

	size_t total_elems = std::accumulate(shapes.begin(), shapes.end(), 0,
			[](const size_t &cur, const tinyobj::shape_t &s){
				return cur + s.mesh.indices.size();
			});
	elem_buf = allocator.alloc(total_elems * sizeof(GLuint), sizeof(GLuint));
	{
		unsigned int *elems = static_cast<unsigned int*>(elem_buf.map(GL_ELEMENT_ARRAY_BUFFER,
					GL_MAP_INVALIDATE_RANGE_BIT | GL_MAP_WRITE_BIT));
		// Track our offset in the element count
		size_t prev_offset = 0;
		for (const auto &s : shapes){
			model_info[s.name] = ModelMatInfo{prev_offset, s.mesh.indices.size()};
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
		for (const auto &s : shapes){
			model_info[s.name].vert_offset = i / 8;
			model_info[s.name].mat_id = s.mesh.material_ids[0];
			for (auto p = s.mesh.positions.begin(), n = s.mesh.normals.begin(), t = s.mesh.texcoords.begin();
					p != s.mesh.positions.end();
					i += 8)
			{
				for (int k = 0; k < 3; ++k, ++p){
					verts[i + k] = *p;
				}
				if (n != s.mesh.normals.end()){
					for (int k = 3; k < 6; ++k, ++n){
						verts[i + k] = *n;
					}
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

	if (!materials.empty()){
		std::set<std::string> texture_files;
		for (const auto &m : materials){
			if (!m.ambient_texname.empty()){
				texture_files.insert(base_path + m.ambient_texname);
			}
			if (!m.diffuse_texname.empty()){
				texture_files.insert(base_path + m.diffuse_texname);
			}
			if (!m.specular_texname.empty()){
				texture_files.insert(base_path + m.specular_texname);
			}
			if (!m.normal_texname.empty()){
				texture_files.insert(base_path + m.normal_texname);
			}
			for (auto it = m.unknown_parameter.begin(); it != m.unknown_parameter.end(); ++it){
				if (it->first == "map_d"){
					texture_files.insert(base_path + it->second);
					break;
				}
			}
		}
		obj_textures = load_texture_set(texture_files);
		if (obj_textures.textures.empty() && !texture_files.empty()){
			return false;
		}

		GLint ssbo_alignment = 0;
		glGetIntegerv(GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT, &ssbo_alignment);
		mat_buf = allocator.alloc(materials.size() * sizeof(Material), ssbo_alignment);
		{
			Material *mats = static_cast<Material*>(mat_buf.map(GL_SHADER_STORAGE_BUFFER,
						GL_MAP_INVALIDATE_RANGE_BIT | GL_MAP_WRITE_BIT));
			for (size_t i = 0; i < materials.size(); ++i){
				const auto &m = materials[i];
				mats[i].ka = glm::vec4(m.ambient[0], m.ambient[1], m.ambient[2], 1);
				mats[i].kd = glm::vec4(m.diffuse[0], m.diffuse[1], m.diffuse[2], 1);
				mats[i].ks = glm::vec4(m.diffuse[0], m.diffuse[1], m.diffuse[2], m.shininess);
				// Find all the texture information used by this material
				if (!m.ambient_texname.empty()){
					const auto &tex = obj_textures.tex_map[base_path + m.ambient_texname];
					mats[i].map_ka_kd.x = tex.first;
					mats[i].map_ka_kd.y = tex.second;
				}
				else {
					mats[i].map_ka_kd.x = -1;
					mats[i].map_ka_kd.y = -1;
				}

				if (!m.diffuse_texname.empty()){
					const auto &tex = obj_textures.tex_map[base_path + m.diffuse_texname];
					mats[i].map_ka_kd.z = tex.first;
					mats[i].map_ka_kd.w = tex.second;
				}
				else {
					mats[i].map_ka_kd.z = -1;
					mats[i].map_ka_kd.w = -1;
				}

				if (!m.specular_texname.empty()){
					const auto &tex = obj_textures.tex_map[base_path + m.specular_texname];
					mats[i].map_ks_n.x = tex.first;
					mats[i].map_ks_n.y = tex.second;
				}
				else {
					mats[i].map_ks_n.x = -1;
					mats[i].map_ks_n.y = -1;
				}

				if (!m.normal_texname.empty()){
					const auto &tex = obj_textures.tex_map[base_path + m.normal_texname];
					mats[i].map_ks_n.z = tex.first;
					mats[i].map_ks_n.w = tex.second;
				}
				else {
					mats[i].map_ks_n.z = -1;
					mats[i].map_ks_n.w = -1;
				}

				// Find the map_d params for alpha cut out textures
				mats[i].map_mask = glm::ivec4{-1};
				for (auto it = m.unknown_parameter.begin(); it != m.unknown_parameter.end(); ++it){
					if (it->first == "map_d"){
						const auto &tex = obj_textures.tex_map[base_path + it->second];
						mats[i].map_mask.x = tex.first;
						mats[i].map_mask.y = tex.second;
						mats[i].map_mask.z = -1;
						mats[i].map_mask.w = -1;
						break;
					}
				}
			}
			mat_buf.unmap(GL_SHADER_STORAGE_BUFFER);
		}
	}
	return true;
}
std::ostream& operator<<(std::ostream &os, const glt::ModelInfo &m){
	os << "glt::ModelInfo:"
		<< "\n\tindex_offset: " << m.index_offset
		<< "\n\tindices: " << m.indices
		<< "\n\tvert_offset: " << m.vert_offset
		<< "\n--------\n";
	return os;
}
std::ostream& operator<<(std::ostream &os, const glt::Material &m){
	os << "Material {\n\tka = " << glm::to_string(m.ka)
		<< "\n\tkd = " << glm::to_string(m.kd)
		<< "\n\tks = " << glm::to_string(m.ks)
		<< "\n\tmap_ka_kd = " << glm::to_string(m.map_ka_kd)
		<< "\n\tmap_ks_n = " << glm::to_string(m.map_ks_n)
		<< "\n\tmap_mask = " << glm::to_string(m.map_mask)
		<< "\n}";
	return os;
}

