#pragma once

#include "frame.h"
#include "lua.hpp"

class ImageDropbox :public Frame{

	std::string target_;

	TextImg image_for_title_;
	TextImg image_for_value_;
public:

	ImageDropbox(Frame* parent, const char* filename, const char* target, const char* default_value);

	~ImageDropbox();

	virtual void draw();

	int procMessage(int msg, int param0, int param1);
};