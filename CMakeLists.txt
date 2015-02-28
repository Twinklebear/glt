cmake_minimum_required(VERSION 2.8)
project(GLT)

# Use modified FindSDL2 and FindGLM that will work with my windows setup
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${GLT_SOURCE_DIR}/cmake")

# Download stb_image and and tinyobjloader
set(EXTERNAL "${GLT_SOURCE_DIR}/external")
set(stb_image_INCLUDE_DIR "${EXTERNAL}/stb_image/")
file(DOWNLOAD "https://raw.githubusercontent.com/nothings/stb/master/stb_image.h"
	"${stb_image_INCLUDE_DIR}/stb_image.h")
set(tinyobj_DIR "${EXTERNAL}/tinyobjloader/")
file(DOWNLOAD "https://raw.githubusercontent.com/syoyo/tinyobjloader/master/tiny_obj_loader.h"
	"${tinyobj_DIR}/include/tiny_obj_loader.h")
file(DOWNLOAD "https://raw.githubusercontent.com/syoyo/tinyobjloader/master/tiny_obj_loader.cc"
	"${tinyobj_DIR}/src/tiny_obj_loader.cc")
set(tinyobj_SRC "${tinyobj_DIR}/src/tiny_obj_loader.cc")
set(tinyobj_INCLUDE_DIR "${tinyobj_DIR}/include")

# Bump up warning levels appropriately for each compiler
if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -std=c++11")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE} -m64 -O3 -march=native")
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
	if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
		string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zi")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /Zi")
	endif()
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")
endif()

add_definitions(-DGLM_FORCE_RADIANS)

find_package(SDL2 REQUIRED)
find_package(GLM REQUIRED)

include_directories(include ${SDL2_INCLUDE_DIR} ${GLM_INCLUDE_DIRS}
	${stb_image_INCLUDE_DIR} ${tinyobj_INCLUDE_DIR})
add_subdirectory(src)

