#include "glt/draw_elems_indirect_cmd.h"


glt::DrawElemsIndirectCmd::DrawElemsIndirectCmd(GLuint count, GLuint instance_count, GLuint first_index,
		GLuint base_vertex, GLuint base_instance)
	: count(count), instance_count(instance_count), first_index(first_index),
	base_vertex(base_vertex), base_instance(base_instance)
{}
std::ostream& operator<<(std::ostream &os, const glt::DrawElemsIndirectCmd &cmd){
	os << "glt::DrawElemsIndirectCmd:"
		<< "\n\tcount: " << cmd.count
		<< "\n\tinstance_count: " << cmd.instance_count
		<< "\n\tfirst_index: " << cmd.first_index
		<< "\n\tbase_vertex: " << cmd.base_vertex
		<< "\n\tbase_instance: "<< cmd.base_instance
		<< "\n------------\n";
	return os;
}

