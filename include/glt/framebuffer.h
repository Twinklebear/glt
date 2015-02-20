#ifndef GLT_FRAMEBUFFER_H
#define GLT_FRAMEBUFFER_H

#include "gl_core_4_5.h"

namespace glt {
/*
 * Checks if the passed framebuffer is complete, returns true if it is
 */
bool check_framebuffer(GLuint fbo);
}

#endif

