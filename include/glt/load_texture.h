#ifndef GLT_TEX_UTIL_H
#define GLT_TEX_UTIL_H

#include <vector>
#include <string>
#include <set>
#include <unordered_map>
#include <utility>
#include "gl_core_4_5.h"

/*
 * Defines various texture loading utility functions
 */
namespace glt {
/*
 * Information about textures used by a group of materials for an obj
 */
struct OBJTextures {
	// The map of texture name to textures index and layer within the texture
	std::unordered_map<std::string, std::pair<GLuint, GLuint>> tex_map;
	// The texture arrays storing all the textures
	std::vector<GLuint> textures;
};
/*
 * Load an image into a 2d texture, creating a new texture id and
 * generating mipmaps. The texture unit desired for the texture should be
 * made active before calling the function. Can optionally pass width and height
 * to get the width and height of the loaded texture
 * returns -1 if loading failed
 */
GLint load_texture_2d(const std::string &file, int *width = nullptr, int *height = nullptr);
/* Load a series of images into a 2d texture array, creating a new texture id
 * The images will appear in the array in the same order they are passed in the vector
 * The texture unit desired for the texture should be made active before calling the function.
 * Can optionally pass width and height to get the width and height of the loaded texture
 * returns -1 if loading failed
 */
GLint load_texture_2d_array(const std::vector<std::string> &files, int *width = nullptr, int *height = nullptr);
/* Load a series of textures into a cubemap. The images will be loaded from individual files
 * onto the faces in the order they're passed, specifically the ordering should be:
 * +X, -X, +Y, -Y, +Z, -Z
 * Can optionally pass width and height to get the width and height of a face of the cubemap
 * returns -1 if loading failed
 */
GLint load_cubemap(const std::vector<std::string> &files, int *width = nullptr, int *height = nullptr);
/*
 * Load the textures passed into as few 2D texture arrays as possible, grouping by image
 * dimension and format. Information about which textures ended up where is returned in the
 * OBJTextures struct
 */
OBJTextures load_texture_set(const std::set<std::string> &files);
}

#endif

