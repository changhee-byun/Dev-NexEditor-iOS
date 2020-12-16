#pragma once

#include "img.h"

class BitmapText :public Img{

	TextureBin* image_;
	int div_w_;
	int div_h_;
	float font_size_w_;
	float font_size_h_;
	float font_spaceing_w_;
	float font_spaceing_h_;
public:
	BitmapText(const char* filename, int div_w, int div_h);

	~BitmapText();

	void setFontsize(float w, float h);

	void setFontSpacing(float w, float h);

	void print(float x, float y, const char* format, ...);

	void setMatrix();

	void draw(int code, float x, float y);
};