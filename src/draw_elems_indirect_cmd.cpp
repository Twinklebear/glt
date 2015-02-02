#include "glt/draw_elems_indirect_cmd.h"


glt::DrawElemsIndirectCmd::DrawElemsIndirectCmd(GLuint count, GLuint instance_count, GLuint first_index,
		GLuint base_vertex, GLuint base_instance)
	: count(count), instance_count(instance_count), first_index(first_index),
	base_vertex(base_vertex), base_instance(base_instance)
{}

