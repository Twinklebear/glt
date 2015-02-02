#ifndef GLT_STRIDED_ARRAY_H
#define GLT_STRIDED_ARRAY_H

namespace glt {
/*
 * The strided array lets you treat an interleaved array as being
 * multiple arrays containing different types with some set stride
 * eg. in an array of mat4, vec4 we can treat the vec4 components
 * as an array of vec4 with a stride of sizeof(mat4) + sizeof(vec4)
 * between each element
 */
template<typename T>
class StridedArray {
	char *array;
	size_t stride;

public:
	/*
	 * Construct the strided array to step between elements with a stride of `stride`
	 */
	StridedArray(char *array, size_t stride) : array(array), stride(stride){}
	T& operator[](const size_t i){
		return *reinterpret_cast<T*>(array + i * stride);
	}
};
}

#endif

