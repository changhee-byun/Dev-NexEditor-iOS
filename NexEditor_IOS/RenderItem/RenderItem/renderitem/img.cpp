#include <stdio.h>
#ifdef DEMONSTRATION
	#include <SDL_ttf.h>
#endif
#include "img.h"
#include "util.h"

GLuint Img::program = 0;
GLuint Img::u_alpha = 0;
GLuint Img::u_matrix = 0;
GLuint Img::u_texture = 0;
GLuint Img::a_pos = 0;
GLuint Img::a_tex = 0;


Img::Img(){


}

Img::~Img(){


}

int Img::invertImage(int pitch, int height, void* image_pixels) { 
   int index; 
   void* temp_row; 
   int height_div_2; 

   temp_row = (void *)malloc(pitch); 
   if(NULL == temp_row) 
   {
#ifdef DEMONSTRATION
      SDL_SetError("Not enough memory for image inversion"); 
#endif//DEMONSTRATION
      return -1; 
   } 
   //if height is odd, don't need to swap middle row 
   height_div_2 = (int) (height * .5); 
   for(index = 0; index < height_div_2; index++)    { 
      //uses string.h 
      memcpy((char *)temp_row, 
		  (char *)(image_pixels)+
         pitch * index, 
         pitch); 

      memcpy( 
		  (char *)(image_pixels)+
         pitch * index, 
		 (char *)(image_pixels)+
         pitch * (height - index-1), 
         pitch); 
      memcpy( 
		  (char *)(image_pixels)+
         pitch * (height - index-1), 
         temp_row, 
         pitch); 
   } 
   free(temp_row); 
   return 0; 
} 

#ifdef DEMONSTRATION
int Img::premultAlpha(SDL_Surface* image)
{
	if (image->format->BytesPerPixel < 4)
		return 0;
	unsigned int* pixels = (unsigned int*)image->pixels;
	int pixel_count = image->pitch / 4;
	for (int h = 0; h < image->h; ++h){
		for (int i = 0; i < pixel_count; ++i){

			unsigned int pixel = *pixels;
			unsigned int alpha_value = ((pixel & image->format->Amask) >> image->format->Ashift) << image->format->Aloss;
			unsigned int red_value = ((pixel & image->format->Rmask) >> image->format->Rshift) << image->format->Rloss;
			unsigned int green_value = ((pixel & image->format->Gmask) >> image->format->Gshift) << image->format->Gloss;
			unsigned int blue_value = ((pixel & image->format->Bmask) >> image->format->Bshift) << image->format->Bloss;

			red_value = (unsigned int)((double)(red_value * alpha_value) / 255.0);
			green_value = (unsigned int)((double)(green_value * alpha_value) / 255.0);
			blue_value = (unsigned int)((double)(blue_value * alpha_value) / 255.0);

			*pixels++ = (red_value << image->format->Rshift)
				| (green_value << image->format->Gshift)
				| (blue_value << image->format->Bshift)
				| (alpha_value << image->format->Ashift);
		}
	}
	return 0;
}

#endif//DEMONSTRATION

void Img::setAlpha(float alpha){

	glUniform1f(u_alpha, alpha); CHECK_GL_ERROR();
}

void Img::setTexture(GLuint tex_id){

	glActiveTexture(GL_TEXTURE0);                                             CHECK_GL_ERROR();
	glBindTexture(GL_TEXTURE_2D, tex_id);                                   CHECK_GL_ERROR();
	glUniform1i(u_texture, 0);                                           CHECK_GL_ERROR();
}

void Img::drawRect(float width, float height){

	float texcoord_buffer[] =
	{
		0, 0,
		0, 1,
		1, 0,
		1, 1,

	};

	float half_w = width * 0.5f;
	float half_h = height * 0.5f;

	float position_buffer[] =
	{
		-half_w, half_h, 0, 1,
		-half_w, -half_h, 0, 1,
		half_w, half_h, 0, 1,
		half_w, -half_h, 0, 1,

	};
	glDisable(GL_CULL_FACE);
	glEnableVertexAttribArray(a_pos); CHECK_GL_ERROR();
	glEnableVertexAttribArray(a_tex); CHECK_GL_ERROR();
	glVertexAttribPointer(a_pos, 4, GL_FLOAT, false, 0, position_buffer);    CHECK_GL_ERROR();
	glVertexAttribPointer(a_tex, 2, GL_FLOAT, false, 0, texcoord_buffer);  CHECK_GL_ERROR();
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);                                      CHECK_GL_ERROR();
}

#ifdef DEMONSTRATION

static GLuint loadImage(SDL_Surface* psurf, float* width, float* height){

	Img::invertImage(psurf->pitch, psurf->h, psurf->pixels);
	Img::premultAlpha(psurf);

	// You should probably use CSurface::OnLoad ... ;)
	//-- and make sure the Surface pointer is good!
	GLuint image;

	glGenTextures(1, &image);
	if (image > 0){
		glBindTexture(GL_TEXTURE_2D, image);

		int Mode = GL_RGB;
		int mode_target;

		if (psurf->format->BytesPerPixel == 4) {
			Mode = GL_RGBA;
			mode_target = 4;
		}
		else{
			Mode = GL_RGB;
			mode_target = 3;
		}

		glTexImage2D(GL_TEXTURE_2D, 0, mode_target, psurf->w, psurf->h, 0, Mode, GL_UNSIGNED_BYTE, psurf->pixels);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		if (width) *width = (float)psurf->w;
		if (height) *height = (float)psurf->h;

		return image;
	}

	return 0;
}

#endif//DEMONSTRATION

#include "soil/SOIL.h"

GLuint Img::loadImage(const char* filename, float* width, float* height){

	int w, h;
	GLuint image = SOIL_load_OGL_texture(filename, SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MULTIPLY_ALPHA | SOIL_FLAG_INVERT_Y | SOIL_FLAG_DDS_LOAD_DIRECT, &w, &h);

	if (width) *width = (float)w;
	if (height) *height = (float)h;

	return image;

}

GLuint Img::loadImage(const char* filename, char* src, int length, float* width, float* height){

	int w, h;
	GLuint image = SOIL_load_OGL_texture_from_memory((const unsigned char *)src, length, 0, 0, SOIL_FLAG_MULTIPLY_ALPHA | SOIL_FLAG_DDS_LOAD_DIRECT, &w, &h);

	if (width) *width = (float)w;
	if (height) *height = (float)h;

	return image;

}

GLuint Img::makeTextImage(const char* text, float* width, float* height, const char* font, int ptsize, int font_style){

#ifdef DEMONSTRATION
	if (!TTF_WasInit()){

		if (-1 == TTF_Init()){

			printf("TTF_Init() Failed:%s\n", TTF_GetError());
			return 0;
		}
	}

	TTF_Font* pfont = TTF_OpenFont(font, ptsize);

	if (!pfont){

		printf("TTF_OpenFont() Failed:%s\n", TTF_GetError());
		return 0;
	}

	TTF_SetFontStyle(pfont, font_style);
	SDL_Color color = { 0, 0, 0, 255 };
	SDL_Surface* psurf = TTF_RenderText_Blended(pfont, text, color);
	if (psurf){

		GLuint image = ::loadImage(psurf, width, height);
		SDL_FreeSurface(psurf);
		return image;
	}
	TTF_CloseFont(pfont);
#endif//DEMONSTRATION
	return 0;
}

GLuint Img::makeTextImageWrapped(const char* text, float* width, float* height, const char* font, int ptsize, int font_style){

#ifdef DEMONSTRATION
	if (!TTF_WasInit()){

		if (-1 == TTF_Init()){

			printf("TTF_Init() Failed:%s\n", TTF_GetError());
			return 0;
		}
	}

	TTF_Font* pfont = TTF_OpenFont(font, ptsize);

	if (!pfont){

		printf("TTF_OpenFont() Failed:%s\n", TTF_GetError());
		return 0;
	}

	TTF_SetFontStyle(pfont, font_style);
	SDL_Color color = { 0, 0, 0, 255 };
	SDL_Surface* psurf = TTF_RenderText_Blended_Wrapped(pfont, text, color, (Uint32)*width);
	if (psurf){

		GLuint image = ::loadImage(psurf, width, height);
		SDL_FreeSurface(psurf);
		return image;
	}
	TTF_CloseFont(pfont);
#endif//DEMONSTRATION
	return 0;
}


void Img::loadShaderForUI(){

	if (program > 0)
		return;

	const char* vertex_shader_for_ui = {
		"#version 120\n"
		"attribute vec4 a_position;\n"
		"attribute vec2 a_texCoord;\n"
		"uniform mat4 u_mvp_matrix;\n"
		"varying highp vec2 v_texCoord;\n"
		"void main() {\n"
		"v_texCoord = a_texCoord;\n"
		"gl_Position = vec4(a_position.xyz, 1) * u_mvp_matrix;\n"
		"}\n"
	};

	const char* fragment_shader_for_ui = {
		"#version 120\n"
		"uniform highp float u_alpha;\n"
		"uniform sampler2D u_textureSampler;\n"
		"varying highp vec2 v_texCoord;\n"
		"\n"
		"void main() {\n"
		"gl_FragColor = (texture2D(u_textureSampler, v_texCoord)).rgba * u_alpha;\n"
		"}\n"
	};


	GLuint vertex_shader = 0, fragment_shader = 0;
	GLint size = 0;

	vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	size = strlen(vertex_shader_for_ui);
	glShaderSource(vertex_shader, 1, &vertex_shader_for_ui, &size);
	glCompileShader(vertex_shader);

	fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	size = strlen(fragment_shader_for_ui);
	glShaderSource(fragment_shader, 1, &fragment_shader_for_ui, &size);
	glCompileShader(fragment_shader);

	program = glCreateProgram(); CHECK_GL_ERROR();
	glAttachShader(program, vertex_shader); CHECK_GL_ERROR();
	glAttachShader(program, fragment_shader); CHECK_GL_ERROR();
	glLinkProgram(program); CHECK_GL_ERROR();

	char error_text[1024];
	GLsizei length = 0;
	glGetProgramInfoLog(program, sizeof(error_text), &length, error_text); CHECK_GL_ERROR();

	u_alpha = glGetUniformLocation(program, "u_alpha"); CHECK_GL_ERROR();
	u_texture = glGetUniformLocation(program, "u_textureSampler"); CHECK_GL_ERROR();
	u_matrix = glGetUniformLocation(program, "u_mvp_matrix"); CHECK_GL_ERROR();
	a_pos = glGetAttribLocation(program, "a_position"); CHECK_GL_ERROR();
	a_tex = glGetAttribLocation(program, "a_texCoord"); CHECK_GL_ERROR();
}
