#include <cmath>
#include <glm/ext.hpp>
#include "glt/flythrough_camera.h"

glt::FlythroughCamera::FlythroughCamera(const glm::mat4 &look_at, float motion_speed, float rotation_speed,
		const std::array<float, 2> &inv_screen)
	: look_at(look_at), translation(glm::mat4{}), rotation(glm::quat{}), camera(look_at),
	inv_camera(glm::inverse(camera)), motion_speed(motion_speed), rotation_speed(rotation_speed),
	inv_screen(inv_screen)
{}
bool glt::FlythroughCamera::mouse_motion(const SDL_MouseMotionEvent &mouse, float elapsed){
	if (mouse.state & SDL_BUTTON_LMASK){
		rotate(mouse, elapsed);
		return true;
	}
	else if (mouse.state & SDL_BUTTON_RMASK){
		pan(mouse, elapsed);
		return true;
	}
	return false;
}
bool glt::FlythroughCamera::mouse_scroll(const SDL_MouseWheelEvent &scroll, float elapsed){
	if (scroll.y != 0){
		glm::vec3 motion{0.f};
		motion.z = scroll.y * 0.35;
		translation = glm::translate(motion * motion_speed * elapsed) * translation;
		camera = translation * look_at * glm::mat4_cast(rotation);
		inv_camera = glm::inverse(camera);
		return true;
	}
	return false;
}
bool glt::FlythroughCamera::keypress(const SDL_KeyboardEvent &key){
	if (key.keysym.sym == SDLK_r){
		translation = glm::mat4{};
		rotation = glm::quat{};
		camera = look_at;
		inv_camera = glm::inverse(camera);
		return true;
	}
	return false;
}
const glm::mat4& glt::FlythroughCamera::transform() const {
	return camera;
}
const glm::mat4& glt::FlythroughCamera::inv_transform() const {
	return inv_camera;
}
glm::vec3 glt::FlythroughCamera::eye_pos() const {
	return glm::vec3{inv_camera * glm::vec4{0, 0, 0, 1}};
}
void glt::FlythroughCamera::rotate(const SDL_MouseMotionEvent &mouse, float elapsed){
	// TODO!
}
void glt::FlythroughCamera::pan(const SDL_MouseMotionEvent &mouse, float elapsed){
	glm::vec3 motion{0.f};
	if (SDL_GetModState() & KMOD_SHIFT){
		motion.z = -mouse.yrel * inv_screen[1] * 2;
	}
	else {
		motion.x = mouse.xrel * inv_screen[0];
		motion.y = -mouse.yrel * inv_screen[1];
	}
	translation = glm::translate(motion * motion_speed * elapsed) * translation;
	camera = translation * look_at * glm::mat4_cast(rotation);
	inv_camera = glm::inverse(camera);
}

