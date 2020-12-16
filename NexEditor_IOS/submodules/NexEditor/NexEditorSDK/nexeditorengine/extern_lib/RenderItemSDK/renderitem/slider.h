#pragma once

#include "frame.h"
#include "lua.hpp"

class SliderBody :public Frame{

	Frame* pslider_;
	float slider_rect_x_;
	float slider_rect_y_;
	float slider_length_;
	int slider_style_;
	float slider_value_;
	float slider_min_value_;
	float slider_max_value_;
	float slider_step_;
private:
	virtual void setPosInternal(float x, float y);

	virtual void addPosinternal(float rel_x, float rel_y);

	void setSliderValue_Internal(float value);

public:
	enum{
		HORIZONTAL,
		VERTICAL,
	};

	SliderBody(Frame* parent, const char* filename);

	~SliderBody();

	virtual bool getArrangePosForRelative(Frame* pframe, float& rel_x, float& rel_y);

	virtual bool getArrangePos(Frame* pframe, float& x, float& y);

	void setSliderLength(float length);

	void setSliderRect(int rel_x, int rel_y);

	void setSliderValue(float value);

	float getSliderValue();

	void setSliderMinValue(float value);

	float getSliderMinValue();

	void setSliderMaxValue(float value);

	float getSliderMaxValue();

	void setSliderStep(float value);

	float getSliderStep();

	void addSlider(Frame* pframe);

	virtual int procMessage(int msg, int param0, int param1);
};

class Slider :public Frame{

	SliderBody* pslider_body_;
	char title_[256];
	char script_target_[256];
	lua_State* lua_state_;

	GLuint image_for_title_;
	float w_for_title_;
	float h_for_title_;

public:

	Slider(Frame* parent, const char* filename);

	virtual void draw();

	void setLuaState(lua_State* L);

	void setTitle(const char* title);

	void setScriptTarget(const char* str);

	void addSlider(SliderBody* pframe);

	virtual int procMessage(int msg, int param0, int param1);
};
