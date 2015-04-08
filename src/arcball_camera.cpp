#include <cmath>
#include <glm/ext.hpp>
#include "glt/arcball_camera.h"

glt::ArcBallCamera::ArcBallCamera(const glm::mat4 &look_at, float motion_speed, float rotation_speed,
		const std::array<float, 2> &inv_screen)
	: look_at(look_at), translation(glm::mat4{}), rotation(glm::quat{}), camera(look_at),
	inv_camera(glm::inverse(camera)), motion_speed(motion_speed), rotation_speed(rotation_speed),
	inv_screen(inv_screen)
{}
bool glt::ArcBallCamera::mouse_motion(const SDL_MouseMotionEvent &mouse, float elapsed){
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
bool glt::ArcBallCamera::mouse_scroll(const SDL_MouseWheelEvent &scroll, float elapsed){
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
bool glt::ArcBallCamera::keypress(const SDL_KeyboardEvent &key){
	if (key.keysym.sym == SDLK_r){
		translation = glm::mat4{};
		rotation = glm::quat{};
		camera = look_at;
		inv_camera = glm::inverse(camera);
		return true;
	}
	return false;
}
const glm::mat4& glt::ArcBallCamera::transform() const {
	return camera;
}
const glm::mat4& glt::ArcBallCamera::inv_transform() const {
	return inv_camera;
}
glm::vec3 glt::ArcBallCamera::eye_pos() const {
	return glm::vec3{inv_camera * glm::vec4{0, 0, 0, 1}};
}
void glt::ArcBallCamera::rotate(const SDL_MouseMotionEvent &mouse, float elapsed){
	using namespace glt;
	// Compute current and previous mouse positions in clip space
	glm::vec2 mouse_cur = glm::vec2{mouse.x * 2.0 * inv_screen[0] - 1.0,
		1.0 - 2.0 * mouse.y * inv_screen[1]};
	glm::vec2 mouse_prev = glm::vec2{(mouse.x - mouse.xrel) * 2.0 * inv_screen[0] - 1.0,
		1.0 - 2.0 * (mouse.y - mouse.yrel) * inv_screen[1]};
	// Clamp mouse positions to stay in screen space range
	mouse_cur = glm::clamp(mouse_cur, glm::vec2{-1, -1}, glm::vec2{1, 1});
	mouse_prev = glm::clamp(mouse_prev, glm::vec2{-1, -1}, glm::vec2{1, 1});
	glm::vec3 mouse_cur_ball = screen_to_arcball(mouse_cur);
	glm::vec3 mouse_prev_ball = screen_to_arcball(mouse_prev);
	glm::vec4 rotate_axis = glm::vec4{glm::normalize(glm::cross(mouse_prev_ball, mouse_cur_ball)), 0};
	// We need to get the rotation axis in world space
	rotate_axis = inv_camera * rotate_axis;
	rotation = glm::rotate(rotation, std::acos(glm::dot(mouse_cur_ball, mouse_prev_ball))
			* rotation_speed * elapsed, glm::vec3{rotate_axis});
	rotation = glm::normalize(rotation);
	camera = translation * look_at * glm::mat4_cast(rotation);
	inv_camera = glm::inverse(camera);
}
void glt::ArcBallCamera::pan(const SDL_MouseMotionEvent &mouse, float elapsed){
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
glm::vec3 glt::screen_to_arcball(const glm::vec2 &p){
	float dist = glm::dot(p, p);
	// If we're on/in the sphere return the point on it
	if (dist <= 1.f){
		return glm::vec3(p, std::sqrt(1.f - dist));
	}
	// otherwise we project the point onto the sphere
	else {
		return glm::vec3{glm::normalize(p), 0};
	}
}

