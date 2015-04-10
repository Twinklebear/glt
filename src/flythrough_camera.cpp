#include <cmath>
#include <glm/ext.hpp>
#include "glt/flythrough_camera.h"

glt::FlythroughCamera::FlythroughCamera(const glm::mat4 &look_at, float motion_speed, float rotation_speed,
		const std::array<float, 2> &inv_screen)
	: look_at(look_at), camera(look_at), inv_camera(glm::inverse(camera)),
	up(inv_camera * glm::vec4{0, 1, 0, 0}), right(inv_camera * glm::vec4{1, 0, 0, 0}),
	dir(inv_camera * glm::vec4{0, 0, -1, 0}), pos(inv_camera * glm::vec4{0, 0, 0, 1}),
	motion_speed(motion_speed), rotation_speed(rotation_speed),
	inv_screen(inv_screen)
{}
bool glt::FlythroughCamera::mouse_motion(const SDL_MouseMotionEvent &mouse, float elapsed){
	if (mouse.state & SDL_BUTTON_LMASK && !(SDL_GetModState() & KMOD_CTRL)){
		rotate(mouse, elapsed);
		inv_camera = glm::inverse(camera);
		return true;
	}
	else if (mouse.state & SDL_BUTTON_RMASK
			|| (mouse.state & SDL_BUTTON_LMASK && SDL_GetModState() & KMOD_CTRL)){
		pan(mouse, elapsed);
		inv_camera = glm::inverse(camera);
		return true;
	}
	return false;
}
bool glt::FlythroughCamera::mouse_scroll(const SDL_MouseWheelEvent &scroll, float elapsed){
	if (scroll.y != 0){
		float motion = motion_speed * elapsed * scroll.y * 0.35f;
		pos += motion * dir;
		camera = glm::lookAt(pos, pos + dir, up);
		return true;
	}
	return false;
}
bool glt::FlythroughCamera::keypress(const SDL_KeyboardEvent &key){
	if (key.keysym.sym == SDLK_r){
		camera = look_at;
		inv_camera = glm::inverse(camera);
		up = glm::vec3{inv_camera * glm::vec4{0, 1, 0, 0}};
	    right = glm::vec3{inv_camera * glm::vec4{1, 0, 0, 0}};
		dir = glm::vec3{inv_camera * glm::vec4{0, 0, -1, 0}};
		pos = glm::vec3{inv_camera * glm::vec4{0, 0, 0, 1}};
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
	// Compute current and previous mouse positions in clip space
	glm::vec2 mouse_cur = glm::vec2{mouse.x * 2.0 * inv_screen[0] - 1.0,
		1.0 - 2.0 * mouse.y * inv_screen[1]};
	glm::vec2 mouse_prev = glm::vec2{(mouse.x - mouse.xrel) * 2.0 * inv_screen[0] - 1.0,
		1.0 - 2.0 * (mouse.y - mouse.yrel) * inv_screen[1]};
	// Clamp mouse positions to stay in screen space range
	mouse_cur = glm::clamp(mouse_cur, glm::vec2{-1, -1}, glm::vec2{1, 1});
	mouse_prev = glm::clamp(mouse_prev, glm::vec2{-1, -1}, glm::vec2{1, 1});
	glm::vec2 delta = mouse_prev - mouse_cur;
	glm::quat rotation;
	rotation = glm::rotate(rotation, delta.x * rotation_speed * elapsed, up);
	rotation = glm::rotate(rotation, delta.y * rotation_speed * elapsed, right);
	up = rotation * up;
	dir = rotation * dir;
	right = glm::cross(dir, up);
	camera = glm::lookAt(pos, pos + dir, up);
}
void glt::FlythroughCamera::pan(const SDL_MouseMotionEvent &mouse, float elapsed){
	glm::vec3 motion{0.f};
	if (SDL_GetModState() & KMOD_SHIFT){
		motion = -mouse.yrel * inv_screen[1] * 2.f * dir;
	}
	else {
		motion = -mouse.xrel * inv_screen[0] * right + mouse.yrel * inv_screen[1] * up;
	}
	motion *= elapsed * motion_speed;
	pos += motion;
	camera = glm::lookAt(pos, pos + dir, up);
}

