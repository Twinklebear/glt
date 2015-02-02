#ifndef GLT_DRAW_ELEMS_INDIRECT_CMD_H
#define GLT_DRAW_ELEMS_INDIRECT_CMD_H

#include "gl_core_4_5.h"

namespace glt {
struct DrawElemsIndirectCmd {
	GLuint count, instance_count, first_index,
		   base_vertex, base_instance;

	DrawElemsIndirectCmd(GLuint count, GLuint instance_count, GLuint first_index,
		   GLuint base_vertex, GLuint base_instance);
};
}

#endif

