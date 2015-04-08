add_library(glt gl_core_4_5.c debug.cpp buffer_allocator.cpp util.cpp draw_elems_indirect_cmd.cpp
	arcball_camera.cpp flythrough_camera.cpp load_models.cpp load_texture.cpp framebuffer.cpp ${tinyobj_SRC})

#install(TARGETS glt DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
#install(DIRECTORY ${GLT_SOURCE_DIR}/include/glt DESTINATION ${CMAKE_INSTALL_PREFIX}/include)

