#ifndef GLT_UTIL_H
#define GLT_UTIL_H

#include <vector>
#include <utility>
#include <string>
#include "gl_core_4_5.h"

namespace glt {
#ifdef _WIN32
const char PATH_SEP = '\\';
#else
const char PATH_SEP = '/';
#endif

inline float to_radians(float deg){
	return deg * 0.01745f;
}
template<typename T>
constexpr inline T clamp(T x, T l, T h){
	return x < l ? l : x > h ? h : x;
}
// Get the resource path for resources located under res/<sub_dir>
// sub_dir defaults to empty to just return res
std::string get_resource_path(const std::string &sub_dir = "");
// Read the contents of a file into the string
std::string get_file_content(const std::string &fname);
// Load a file's content and its includes returning the file with includes inserted
// and #line directives for better GLSL error messages within the included files
// the vector of file names will be filled with the file name for each file name number
// in the #line directive
std::string load_shader_file(const std::string &fname, std::vector<std::string> &file_names);
// Load a GLSL shader from the file. Returns -1 if loading fails and prints
// out the compilation errors
GLint load_shader(GLenum type, const std::string &file);
// Load a GLSL shader program from the shader files specified. The pair
// to specify a shader is { shader type, shader file }
// Returns -1 if program creation fails
GLint load_program(const std::vector<std::pair<GLenum, std::string>> &shader_files);
}

#endif

