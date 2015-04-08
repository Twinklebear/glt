#ifndef GLT_FLYTHROUGH_CAMERA_H
#define GLT_FLYTHROUGH_CAMERA_h

#include <array>
#include <SDL.h>
#include <glm/glm.hpp>

namespace glt {
/*
 * A simple flythrough camera
 * Controls:
 * left mouse + drag: pan camera
 * right mouse + drag: translate camera
 * shift + right mouse + drag: zoom in/out
 * scroll: zoom in/out
 * r key: reset camera to original look_at matrix
 */
class FlythroughCamera {
	// We store the unmodified look at matrix along with
	// decomposed translation and rotation components
	glm::mat4 look_at, translation;
	glm::quat rotation;
	// camera is the full camera transform,
	// inv_camera is stored as well to easily compute
	// eye position and world space rotation axes
	glm::mat4 camera, inv_camera;
	// Motion and rotation speeds
	float motion_speed, rotation_speed;
	// Inverse x, y window dimensions
	std::array<float, 2> inv_screen;

public:
	/*
	 * Create an arcball camera with some look at matrix
	 * motion speed: units per second speed of panning the camera
	 * rotation speed: radians per second speed of rotation the camera
	 * inv_screen: { 1 / WIN_X_SIZE, 1 / WIN_Y_SIZE }
	 */
	FlythroughCamera(const glm::mat4 &look_at, float motion_speed, float rotation_speed,
			const std::array<float, 2> &inv_screen);
	/*
	 * Handle mouse motion events to move the camera
	 * returns true if the camera has moved
	 */
	bool mouse_motion(const SDL_MouseMotionEvent &mouse, float elapsed);
	/*
	 * Handle mouse scroll events to zoom in/out
	 * returns true if the camera has moved
	 */
	bool mouse_scroll(const SDL_MouseWheelEvent &scroll, float elapsed);
	/*
	 * Handle keyboard events to reset the camera
	 * returns true if the camera has moved
	 */
	bool keypress(const SDL_KeyboardEvent &key);
	/*
	 * Get the camera transformation matrix
	 */
	const glm::mat4& transform() const;
	/*
	 * Get the camera inverse transformation matrix
	 */
	const glm::mat4& inv_transform() const;
	/*
	 * Get the eye position of the camera in world space
	 */
	glm::vec3 eye_pos() const;

private:
	/*
	 * Handle rotation events
	 */
	void rotate(const SDL_MouseMotionEvent &mouse, float elapsed);
	/*
	 * Handle panning/zooming events
	 */
	void pan(const SDL_MouseMotionEvent &mouse, float elapsed);
};
}

#endif

