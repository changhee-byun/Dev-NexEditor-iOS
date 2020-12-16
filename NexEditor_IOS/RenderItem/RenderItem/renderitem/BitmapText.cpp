#include <stdio.h>
#include "BitmapText.h"
#include "NexTheme_Math.h"
#include "util.h"

BitmapText::BitmapText(const char* filename, int div_w, int div_h) :font_size_w_(0), font_size_h_(0), image_(NULL){

	if (!filename)
		return;

	loadShaderForUI();
	ResourceManager<TextureBin>& manager = *(Singleton<ResourceManager<TextureBin> >::getInstance());
	image_ = manager.get(std::string(filename));

	if (NULL == image_){

		image_ = new TextureBin();
		image_->setTexID(Img::loadImage(filename, &font_size_w_, &font_size_h_));
		image_->setWidth((int)font_size_w_);
		image_->setHeight((int)font_size_h_);
		manager.insert(std::string(filename), image_);
	}
	else{

		font_size_w_ = (float)image_->getWidth();
		font_size_h_ = (float)image_->getHeight();
	}

	image_->addRef();
		
	font_size_w_ /= div_w;
	font_size_h_ /= div_h;
}

BitmapText::~BitmapText(){

	if (image_)
		image_->decRef();
}

void BitmapText::setFontsize(float w, float h){

	font_size_w_ = w;
	font_size_h_ = h;
}

void BitmapText::setFontSpacing(float w, float h){

	font_spaceing_w_ = w;
	font_spaceing_h_ = h;
}

void BitmapText::draw(int code, float x, float y){

	int left_order = code % div_w_;
	int top_order = (code - left_order) / div_w_;
	float tex_w = 1.0f / (float)div_w_;
	float tex_h = 1.0f / (float)div_h_;
	float target_w = tex_w * (float)left_order;
	float target_h = 1.0f - tex_h * (float)top_order - tex_h;
	tex_h *= 0.99f;

	float texcoord_buffer[] =
	{
		target_w, target_h,
		target_w, target_h + tex_h,
		target_w + tex_w, target_h,
		target_w + tex_w, target_h + tex_h,
	};

	float position_buffer[] =
	{
		x, y + font_size_h_, 0, 1,
		x, y, 0, 1,
		x + font_size_w_, y + font_size_h_, 0, 1,
		x + font_size_w_, y, 0, 1,

	};
	glDisable(GL_CULL_FACE);
	glEnableVertexAttribArray(a_pos); CHECK_GL_ERROR();
	glEnableVertexAttribArray(a_tex); CHECK_GL_ERROR();
	glVertexAttribPointer(a_pos, 4, GL_FLOAT, false, 0, position_buffer);    CHECK_GL_ERROR();
	glVertexAttribPointer(a_tex, 2, GL_FLOAT, false, 0, texcoord_buffer);  CHECK_GL_ERROR();
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);                                      CHECK_GL_ERROR();
}

void BitmapText::print(float x, float y, const char* format, ...){

	if (NULL == image_)
		return;
	char buf[512];
	va_list args;
	int len = 0;

	va_start(args, format);
	len = vsnprintf(buf, sizeof(buf), format, args);
	va_end(args);
	if (len > 0){

		glUseProgram(program);
		setMatrix();
		setAlpha(1.0f);
		setTexture(image_->getTexID());
		for (int i = 0; i < len; ++i){

			draw(buf[i], x, y);
			x += font_spaceing_w_;
		}
	}
}

void BitmapText::setMatrix(){

	RITM_NXT_Matrix4f matrix;
	matrix = RITM_NXT_Matrix4f_Ortho(0.0f, 1280.0f, 720.0f, 0.0f, 0.0f, 3000.0f);
	glUniformMatrix4fv(u_matrix, 1, false, matrix.e);
}