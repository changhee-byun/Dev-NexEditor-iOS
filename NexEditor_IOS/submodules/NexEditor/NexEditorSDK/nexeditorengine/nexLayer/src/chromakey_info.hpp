#pragma once

struct ChromakeyInfo{

	float di0;
	float di1;
	float di2;
	float di3;

	float s1;
	float s2;
	float s3;

	float r;
	float g;
	float b;

	int red;
	int green;
	int blue;

	void set(int color, float clipFg, float clipBg, float bx0, float by0, float bx1, float by1){

		clipBg = clipBg*clipBg;
		clipFg = clipFg*clipFg;

		float gap_ = clipFg - clipBg;

		di0 = clipBg;
		di1 = di0 + bx0 * gap_;
		di2 = di0 + bx1 * gap_;
		di3 = clipFg;

		s1 = by0;
		s2 = by1 - by0;
		s3 = 1.0f - by1;

		red = (color >> 16) & 0xff;
		green = (color >> 8) & 0xff;
		blue = (color >> 0) & 0xff;

		r = (float)red / 255.0f;
		g = (float)green / 255.0f;
		b = (float)blue / 255.0f;
	}
};