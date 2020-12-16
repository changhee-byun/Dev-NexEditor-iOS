#pragma once

#define NO_SDL_GLEXT
#include "glext_loader.h"
#include "util.h"

class Img{

protected:
	static GLuint program;
	static GLuint u_alpha;
	static GLuint u_texture;
	static GLuint u_matrix;
	static GLuint a_pos;
	static GLuint a_tex;
	
	static void loadShaderForUI();
public:
	Img();

	~Img();

	static GLuint loadImage(const char* filename, float* width, float* height);

	static GLuint loadImage(const char* filename, char* src, int length, float* width, float* height);

	static GLuint loadImageRaw(const char* pixels, int width, int height);

	static GLuint makeTextImage(const char* text, float* width, float* height, const char* font, int ptsize, int font_style);

	static GLuint makeTextImageWrapped(const char* text, float* width, float* height, const char* font, int ptsize, int font_style);

	static void setAlpha(float alpha);

	static void setTexture(GLuint tex_id);

	static void drawRect(float width, float height);

	static int invertImage(int pitch, int height, void* image_pixels);

#ifdef DEMONSTRATION
	static int premultAlpha(SDL_Surface* image);
#endif//DEMONSTRATION 
};