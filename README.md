GLT - A Small OpenGL Toolkit
---
Some utilities I've written to make working with OpenGL a bit easier. Provides a simple allocator for allocating subregions
of a large GL buffer for various uses. Also has some texture loading support via [stb\_image](https://github.com/nothings/stb)
and basic support for loading models packed into a buffer and element buffer pair via [tinyobjloader](https://github.com/syoyo/tinyobjloader).
OpenGL function loading support is also included in gl\_core\_4\_5(.c/.h) which is generated by glLoadGen, but you can replace these with any loader
you prefer. The library also depends on SDL2 and GLM, stb\_image and tinyobjloader are downloaded automatically by CMake when building the library.

