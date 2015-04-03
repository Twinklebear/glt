#include <iostream>
#include <cmath>
#include <utility>
#include <map>
#include <cassert>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "glt/load_texture.h"

//Swap rows of n bytes pointed to by a with those pointed to by b
//for use in doing the y-flip for images so OpenGL has them right-side up
void swap_row(unsigned char *a, unsigned char *b, size_t n){
	for (size_t i = 0; i < n; ++i){
		std::swap(a[i], b[i]);
	}
}
GLint glt::load_texture_2d(const std::string &file, int *width, int *height){
	int x, y, n;
	unsigned char *img = stbi_load(file.c_str(), &x, &y, &n, 0);
	if (!img){
		std::cout << "load_texture_2d error loading " << file
			<< " - " << stbi_failure_reason() << std::endl;
		return -1;
	}
	if (width){
		*width = x;
	}
	if (height){
		*height = y;
	}
	GLenum format;
	switch (n){
		case 1:
			format = GL_RED;
			break;
		case 2:
			format = GL_RG;
			break;
		case 3:
			format = GL_RGB;
			break;
		default:
			format = GL_RGBA;
			break;
	}
	for (int i = 0; i < y / 2; ++i){
		swap_row(&img[i * x * n], &img[(y - i - 1) * x * n], x * n);
	}
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, format, x, y, 0, format, GL_UNSIGNED_BYTE, img);
	glGenerateMipmap(GL_TEXTURE_2D);
	stbi_image_free(img);
	return tex;
}
GLint glt::load_texture_2d_array(const std::vector<std::string> &files, int *width, int *height){
	assert(!files.empty());
	int x, y, n;
	std::vector<unsigned char*> images;
	//We need to load the first image to get the dimensions and format we're loading
	images.push_back(stbi_load(files.front().c_str(), &x, &y, &n, 0));
	for (auto f = ++files.begin(); f != files.end(); ++f){
		int ix, iy, in;
		unsigned char *im = stbi_load(f->c_str(), &ix, &iy, &in, 0);
		bool error = false;
		if (!im){
			std::cout << "load_texture_2d_array error loading " << *f
				<< " - " << stbi_failure_reason() << std::endl;
			error = true;
		}
		if (x != ix || y != iy || n != in){
			std::cout << "load_texture_2d_array error: incompatible file types found on "
				<< *f << std::endl;
			error = true;
		}
		if (error){
			for (auto i : images){
				stbi_image_free(i);
			}
			return -1;
		}
		images.push_back(im);
	}
	if (width){
		*width = x;
	}
	if (height){
		*height = y;
	}
	//Perform y-swap on each loaded image
	for (auto img : images){
		for (int i = 0; i < y / 2; ++i){
			swap_row(&img[i * x * n], &img[(y - i - 1) * x * n], x * n);
		}
	}
	GLenum format;
	switch (n){
		case 1:
			format = GL_RED;
			break;
		case 2:
			format = GL_RG;
			break;
		case 3:
			format = GL_RGB;
			break;
		default:
			format = GL_RGBA;
			break;
	}
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D_ARRAY, tex);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, format, x, y, images.size(), 0, format, GL_UNSIGNED_BYTE, NULL);
	//Upload all the textures in the array
	for (size_t i = 0; i < images.size(); ++i){
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, x, y, 1, format, GL_UNSIGNED_BYTE, images.at(i));
	}
	glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
	//Clean up all the image data
	for (auto i : images){
		stbi_image_free(i);
	}
	return tex;
}
GLint glt::load_cubemap(const std::vector<std::string> &files, int *width, int *height){
	assert(files.size() == 6);
	int x, y, n;
	std::vector<unsigned char*> images;
	//We need to load the first image to get the dimensions and format we're loading
	unsigned char *im = stbi_load(files.front().c_str(), &x, &y, &n, 0);
	if (!im){
		std::cout << "load_cubemap error loading " << files.front()
			<< " - " << stbi_failure_reason() << std::endl;
		return -1;
	}
	images.push_back(im);
	for (auto f = ++files.begin(); f != files.end(); ++f){
		int ix, iy, in;
		unsigned char *im = stbi_load(f->c_str(), &ix, &iy, &in, 0);
		bool error = false;
		if (!im){
			std::cout << "load_cubemap error loading " << *f
				<< " - " << stbi_failure_reason() << std::endl;
			error = true;
		}
		if (x != ix || y != iy || n != in){
			std::cout << "load_cubemap error: incompatible file types found on "
				<< *f << std::endl;
			error = true;
		}
		if (error){
			for (auto i : images){
				stbi_image_free(i);
			}
			return -1;
		}
		images.push_back(im);
	}
	if (width){
		*width = x;
	}
	if (height){
		*height = y;
	}
	// Perform y-swap on each loaded image
	for (auto img : images){
		for (int i = 0; i < y / 2; ++i){
			swap_row(&img[i * x * n], &img[(y - i - 1) * x * n], x * n);
		}
	}
	GLenum format;
	switch (n){
		case 1:
			format = GL_RED;
			break;
		case 2:
			format = GL_RG;
			break;
		case 3:
			format = GL_RGB;
			break;
		default:
			format = GL_RGBA;
			break;
	}
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
	//Upload all the textures in the array
	for (size_t i = 0; i < images.size(); ++i){
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, x, y, 0, format,
				GL_UNSIGNED_BYTE, images[i]);
	}
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	//Clean up all the image data
	for (auto i : images){
		stbi_image_free(i);
	}
	return tex;
}

/* 
 * Information about the image dimensions and channels of a texture so we
 * can group matching textures into arrays
 */
struct TextureInfo {
	int width, height, channels;
};
bool operator<(const TextureInfo &a, const TextureInfo &b){
	if (a.width == b.width){
		if (a.height == b.height){
			if (a.channels == b.channels){
				return false;
			}
			return a.channels < b.channels;
		}
		return a.height < b.height;
	}
	return a.width < b.width;
}
std::ostream& operator<<(std::ostream &os, const TextureInfo &t){
	os << "TextureInfo {\n\twidth = " << t.width
		<< "\n\theight = " << t.height
		<< "\n\tchannels = " << t.channels
		<< "\n}";
	return os;
}

struct Texture {
	TextureInfo info;
	std::string name;
	unsigned char *tex;
};

glt::OBJTextures glt::load_texture_set(const std::set<std::string> &files){
	OBJTextures obj_textures;
	std::map<TextureInfo, std::vector<Texture>> unique_textures;
	for (auto f = files.begin(); f != files.end(); ++f){
		Texture tex;
		tex.name = *f;
		tex.tex = stbi_load(f->c_str(), &tex.info.width, &tex.info.height, &tex.info.channels, 0);
		if (!tex.tex){
			std::cout << "load_texture_set error loading " << *f
				<< " - " << stbi_failure_reason() << std::endl;
			for (auto &u : unique_textures){
				for (auto &t : u.second){
					stbi_image_free(t.tex);
				}
			}
			return obj_textures;
		}
		// Perform y-swap on loaded images
		for (int i = 0; i < tex.info.height / 2; ++i){
			swap_row(&tex.tex[i * tex.info.width * tex.info.channels],
					&tex.tex[(tex.info.height - i - 1) * tex.info.width * tex.info.channels],
					tex.info.width * tex.info.channels);
		}
		unique_textures[tex.info].push_back(tex);
	}
	// Upload each texture array and track where the textures ended up in obj_textures
	for (const auto &u : unique_textures){
		const TextureInfo &info = u.first;
		GLenum format, sized_format;
		switch (info.channels){
			case 1:
				format = GL_RED;
				sized_format = GL_R8;
				break;
			case 2:
				format = GL_RG;
				sized_format = GL_RG8;
				break;
			case 3:
				format = GL_RGB;
				sized_format = GL_RGB8;
				break;
			default:
				format = GL_RGBA;
				sized_format = GL_RGBA8;
				break;
		}
		GLuint tex;
		glGenTextures(1, &tex);
		glBindTexture(GL_TEXTURE_2D_ARRAY, tex);
		const GLsizei levels = std::log2(std::max(info.width, info.height));
		glTexStorage3D(GL_TEXTURE_2D_ARRAY, levels, sized_format, info.width, info.height, u.second.size());
		for (size_t i = 0; i < u.second.size(); ++i){
			glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, info.width, info.height,
					1, format, GL_UNSIGNED_BYTE, u.second[i].tex);
			obj_textures.tex_map[u.second[i].name] = std::make_pair(obj_textures.textures.size(), i);
		}
		glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
		obj_textures.textures.push_back(tex);
	}
	for (auto &u : unique_textures){
		for (auto &t : u.second){
			stbi_image_free(t.tex);
		}
	}
	return obj_textures;
}

