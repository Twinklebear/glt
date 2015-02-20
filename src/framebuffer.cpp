#include <iostream>
#include "glt/framebuffer.h"

bool glt::check_framebuffer(GLuint fbo){
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	switch (status){
		case GL_FRAMEBUFFER_UNDEFINED:
			std::cout << "check_framebuffer - fbo incomplete: undefined framebuffer\n";
			return false;
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
			std::cout << "check_framebuffer - fbo incomplete: attachment incomplete\n";
			return false;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
			std::cout << "check_framebuffer - fbo incomplete: missing attachment\n";
			return false;
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
			std::cout << "check_framebuffer - fbo incomplete: a draw buffer is incomplete\n";
			return false;
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
			std::cout << "check_framebuffer - fbo incomplete: a read buffer is incomplete\n";
			return false;
		case GL_FRAMEBUFFER_UNSUPPORTED:
			std::cout << "check_framebuffer - fbo incomplete: framebuffer configuration is unsupported\n";
			return false;
		case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
			std::cout << "check_framebuffer - fbo incomplete: multisample configuration does not match"
				<< " for all attachments\n";
			return false;
		case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
			std::cout << "check_framebuffer - fbo incomplete: layer targets are incomplete\n";
			return false;
		default:
			break;
	}
	return true;
}

