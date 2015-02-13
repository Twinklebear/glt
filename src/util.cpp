#include <iostream>
#include <algorithm>
#include <vector>
#include <utility>
#include <string>
#include <fstream>
#include <SDL.h>
#include "glt/gl_core_4_5.h"
#include "glt/util.h"

std::string glt::get_resource_path(const std::string &sub_dir){
	using namespace glt;
	static std::string base_res;
	if (base_res.empty()){
		char *base_path = SDL_GetBasePath();
		if (base_path){
			base_res = base_path;
			SDL_free(base_path);
		}
		else {
			std::cout << "Error getting resource path: " << SDL_GetError() << std::endl;
			return "";
		}
		// The executable will be located under bin/ so replace it with res to
		// get the resouce folder
		size_t pos = base_res.rfind("bin");
		base_res = base_res.substr(0, pos) + "res" + PATH_SEP;
	}
	if (sub_dir.empty()){
		return base_res;
	}
	return base_res + sub_dir + PATH_SEP;
}
std::string glt::get_file_content(const std::string &fname){
	std::ifstream file{fname};
	if (!file.is_open()){
		std::cout << "Failed to open file: " << fname << std::endl;
		return "";
	}
	return std::string{std::istreambuf_iterator<char>{file},
		std::istreambuf_iterator<char>{}};
}
std::string glt::load_shader_file(const std::string &fname, std::vector<std::string> &file_names){
	if (std::find(file_names.begin(), file_names.end(), fname) != file_names.end()){
		std::cout << "Multiple includes of file " << fname << " detected, dropping this include\n";
		return "";
	}
	std::string content = get_file_content(fname);
	file_names.push_back(fname);
	// Insert the current file name index and line number for this file to preserve error logs
	// before inserting the included file contents
	size_t inc = content.rfind("#include");
	if (inc != std::string::npos){
		size_t line_no = std::count_if(content.begin(), content.begin() + inc,
				[](const char &c){ return c == '\n'; });
		content.insert(content.find("\n", inc) + 1, "#line " + std::to_string(line_no + 2)
				+ " " + std::to_string(file_names.size() - 1) + "\n");
	}
	else if (file_names.size() > 1){
		content.insert(0, "#line 1 " + std::to_string(file_names.size() - 1) + "\n");
	}
	std::string dir = fname.substr(0, fname.rfind(PATH_SEP) + 1);
	// Insert includes backwards so we don't waste time parsing through the inserted file after inserting
	for (; inc != std::string::npos; inc = content.rfind("#include", inc - 1)){
		size_t open = content.find("\"", inc + 8);
		size_t close = content.find("\"", open + 1);
		std::string included = content.substr(open + 1, close - open - 1);
		content.erase(inc, close - inc + 2);
		std::string include_content = load_shader_file(dir + included, file_names);
		if (!include_content.empty()){
			content.insert(inc, include_content);
		}
	}
	return content;
}
// Extract the file number the error occured in from the log message
// it's expected that the message begins with file_no(line_no)
// TODO: Is this always the form of the compilation errors?
// TODO it's not always the form of the compilation errors, need to
// handle intel and possible AMD differences properly
int get_file_num(const std::vector<char> &log){
	auto paren = std::find(log.begin(), log.end(), '(');
	std::string file_no{log.begin(), paren};
	//return std::stoi(file_no);
	return 0;
}
GLint glt::load_shader(GLenum type, const std::string &file){
	GLuint shader = glCreateShader(type);
	std::vector<std::string> file_names;
	std::string src = glt::load_shader_file(file, file_names);
	const char *csrc = src.c_str();
	glShaderSource(shader, 1, &csrc, 0);
	glCompileShader(shader);
	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE){
		std::cout << "Shader compilation error, ";
		switch (type){
			case GL_VERTEX_SHADER:
				std::cout << "Vertex shader: ";
				break;
			case GL_FRAGMENT_SHADER:
				std::cout << "Fragment shader: ";
				break;
			case GL_GEOMETRY_SHADER:
				std::cout << "Geometry shader: ";
				break;
			case GL_COMPUTE_SHADER:
				std::cout << "Compute shader: ";
				break;
			case GL_TESS_CONTROL_SHADER:
				std::cout << "Tessellation Control shader: ";
				break;
			case GL_TESS_EVALUATION_SHADER:
				std::cout << "Tessellation Evaluation shader: ";
				break;
			default:
				std::cout << "Unknown shader type: ";
		}
		std::cout << file << " failed to compile. Compilation log:\n";
		GLint len;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
		std::vector<char> log(len, '\0');
		log.resize(len);
		glGetShaderInfoLog(shader, log.size(), 0, log.data());
		std::cout << "In file: " << file_names[get_file_num(log)] << ":\n" << log.data() << "\n";
		glDeleteShader(shader);
		return -1;
	}
	return shader;
}
GLint glt::load_program(const std::vector<std::pair<GLenum, std::string>> &shader_files){
	std::vector<GLuint> shaders;
	for (const auto &s : shader_files){
		GLint h = load_shader(std::get<0>(s), std::get<1>(s));
		if (h == -1){
			std::cout << "Error loading shader program: A required shader failed to compile, aborting\n";
			for (GLuint g : shaders){
				glDeleteShader(g);
			}
			return -1;
		}
		shaders.push_back(h);
	}
	GLuint program = glCreateProgram();
	for (const auto &s : shaders){
		glAttachShader(program, s);
	}
	glLinkProgram(program);
	GLint status;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (status == GL_FALSE){
		std::cout << "Error loading shader program: Program failed to link, log:\n";
		GLint len;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
		std::vector<char> log(len, '\0');
		log.resize(len);
		glGetProgramInfoLog(program, log.size(), 0, log.data());
		std::cout << log.data() << "\n";
	}
	for (GLuint s : shaders){
		glDetachShader(program, s);
		glDeleteShader(s);
	}
	if (status == GL_FALSE){
		glDeleteProgram(program);
		return -1;
	}
	return program;
}

