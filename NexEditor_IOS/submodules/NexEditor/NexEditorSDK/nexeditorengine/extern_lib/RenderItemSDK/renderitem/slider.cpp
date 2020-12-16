#include "slider.h"
#include "glext_loader.h"
#include <math.h>
#include "nexTexturesLogger.h"

void SliderBody::setPosInternal(float x, float y){

	float rel_x = x - x_;
	float rel_y = y - y_;
	addPosinternal(rel_x, rel_y);
}

void SliderBody::addPosinternal(float rel_x, float rel_y){

	x_ += rel_x;
	y_ += rel_y;
	slider_rect_x_ += rel_x;
	slider_rect_y_ += rel_y;
}

SliderBody::SliderBody(Frame* parent, const char* filename) :Frame(parent, filename), pslider_(NULL){

	slider_rect_x_ = 0.0f;
	slider_rect_y_ = 0.0f;
	slider_length_ = 0.0f;
	slider_style_ = HORIZONTAL;
	slider_value_ = 0.0f;
}

SliderBody::~SliderBody(){


}

bool SliderBody::getArrangePosForRelative(Frame* pframe, float& rel_x, float& rel_y){

	if (!pslider_)
		return true;

	if (pslider_ != pframe)
		return true;

	if (slider_style_ == HORIZONTAL){

		rel_y = slider_rect_y_ - pslider_->getPosY() - (pslider_->getHeight() * 0.5f);
		float x = pslider_->getPosX() + rel_x + (pslider_->getWidth() * 0.5f);
		if (x < slider_rect_x_)
			x = slider_rect_x_;
		else if (x > (slider_rect_x_ + slider_length_))
			x = (slider_rect_x_ + slider_length_);
		rel_x = x - pslider_->getPosX() - (pslider_->getWidth() * 0.5f);
	}
	else if (slider_style_ == VERTICAL){

		rel_x = slider_rect_x_ - pslider_->getPosX() - (pslider_->getWidth() * 0.5f);
		float y = pslider_->getPosY() + rel_y;
		if (y < slider_rect_y_)
			y = slider_rect_y_;
		else if (y > slider_rect_y_ + slider_length_)
			y = slider_rect_y_ + slider_length_;
		rel_y = y - pslider_->getPosY() - (pslider_->getHeight() * 0.5f);
	}
	return true;
}

bool SliderBody::getArrangePos(Frame* pframe, float& x, float& y){

	if (!pslider_)
		return true;

	if (pslider_ != pframe)
		return true;

	if (slider_style_ == HORIZONTAL){

		y = slider_rect_y_ - pslider_->getHeight() * 0.5f;
		if (x < slider_rect_x_ - pslider_->getWidth() * 0.5f)
			x = slider_rect_x_ - pslider_->getWidth() * 0.5f;
		else if (x > slider_rect_x_ + slider_length_ - pslider_->getWidth() * 0.5f)
			x = slider_rect_x_ + slider_length_ - pslider_->getWidth() * 0.5f;
	}
	else if (slider_style_ == VERTICAL){

		x = slider_rect_x_ - pslider_->getWidth() * 0.5f;
		if (y < slider_rect_y_)
			y = slider_rect_y_;
		else if (y > slider_rect_y_ + slider_length_)
			y = slider_rect_y_ + slider_length_;
	}

	return true;
}

void SliderBody::setSliderLength(float length){

	slider_length_ = length;
}

void SliderBody::setSliderRect(int rel_x, int rel_y){

	slider_rect_x_ = x_ + rel_x;
	slider_rect_y_ = y_ + rel_y;

	if (pslider_){

		float x = slider_rect_x_ - pslider_->getWidth() * 0.5f;
		float y = slider_rect_y_ - pslider_->getHeight() * 0.5f;

		if (slider_style_ == HORIZONTAL){

			x = x + slider_length_ * slider_value_;
		}
		else{

			y = y + slider_length_ * slider_value_;
		}

		pslider_->setPos(x, y);
	}
}

void SliderBody::setSliderValue_Internal(float value){

	slider_value_ = value;

	if (parent_){

		parent_->procMessage(USER_MSG_UPDATED, (int)this, 0);
	}
}

void SliderBody::setSliderValue(float value){

	if (!pslider_){

		setSliderValue_Internal(value);
		return;
	}

	float x = slider_rect_x_ - pslider_->getWidth() * 0.5f;
	float y = slider_rect_y_ - pslider_->getHeight() * 0.5f;

	if (slider_style_ == HORIZONTAL){

		x = x + slider_length_ * value / (slider_max_value_ - slider_min_value_);
	}
	else{

		y = y + slider_length_ * value / (slider_max_value_ - slider_min_value_);
	}

	pslider_->setPos(x, y);

	//Currently the move of child revoke the setSliderValue_internal, 
	//So, we don't call the function explicitly.
	//But, it is not good programming, so I should think over it.
}

float SliderBody::getSliderValue(){

	return slider_value_;
}

void SliderBody::setSliderMinValue(float value){

	slider_min_value_ = value;
}

float SliderBody::getSliderMinValue(){

	return slider_min_value_;
}

void SliderBody::setSliderMaxValue(float value){

	slider_max_value_ = value;
}

float SliderBody::getSliderMaxValue(){

	return slider_max_value_;
}

void SliderBody::setSliderStep(float value){

	slider_step_ = value;
}

float SliderBody::getSliderStep(){

	return slider_step_;
}

void SliderBody::addSlider(Frame* pframe){

	pslider_ = pframe;

	float x = slider_rect_x_ - pslider_->getWidth() * 0.5f;
	float y = slider_rect_y_ - pslider_->getHeight() * 0.5f;

	if (slider_style_ == HORIZONTAL){

		x = x + slider_length_ * slider_value_ / (slider_max_value_ - slider_min_value_);
	}
	else{

		y = y + slider_length_ * slider_value_ / (slider_max_value_ - slider_min_value_);
	}

	pslider_->setPos(x, y);
	addChild(pslider_);
}

int SliderBody::procMessage(int msg, int param0, int param1){

	if (!pslider_)
		return 0;

	if (msg == USER_MSG_CLICK){

		float x = (float)param0;
		float y = (float)param1;
		if (slider_style_ == HORIZONTAL){

			if (fabsf(y - slider_rect_y_) < 3.0f){

				float slider_center = slider_rect_x_ + slider_length_ * 0.5f;
				if (fabsf(x - slider_center) < slider_length_ * 0.5f){

					pslider_->addPos(x - pslider_->getPosX() - pslider_->getWidth() * 0.5f, 0.0f);
				}
			}
		}
		else if (slider_style_ == VERTICAL){

			if (fabsf(x - slider_rect_x_) < 3.0f){

				float slider_center = slider_rect_y_ + slider_length_ * 0.5f;
				if (fabsf(y - slider_center) < slider_length_ * 0.5f){

					pslider_->addPos(x - pslider_->getPosX() - pslider_->getWidth() * 0.5f, 0.0f);
					pslider_->addPos(0.0f, y - -pslider_->getPosY() - pslider_->getHeight() * 0.5f);
				}
			}
		}
	}
	if (msg == USER_MSG_MOVE_CHILD){

		float x = (float)param0;
		float y = (float)param1;
		float val = 0.0f;
		if (slider_style_ == HORIZONTAL){

			val = x + pslider_->getWidth() * 0.5f - slider_rect_x_;
		}
		else if (slider_style_ == VERTICAL){

			val = y + pslider_->getHeight() * 0.5f - slider_rect_y_;
		}

		val = val / slider_length_ * (slider_max_value_ - slider_min_value_) + slider_min_value_;
		setSliderValue_Internal(val);
	}
	return 0;
}


Slider::Slider(Frame* parent, const char* filename) :Frame(parent, filename), pslider_body_(NULL), lua_state_(NULL), image_for_title_(0){

	title_[0] = 0;
	script_target_[0] = 0;
}

void Slider::draw(){

	Frame::draw();
	//To Do
	if (image_for_title_ <= 0)
		return;

	glUseProgram(program);
	setMatrix(x_, y_, w_for_title_, h_for_title_);
	setAlpha(1.0f);
	setTexture(image_for_title_);
	drawRect(w_for_title_, h_for_title_);	
}

void Slider::setLuaState(lua_State* L){

	lua_state_ = L;
}

void Slider::setTitle(const char* title){

	strcpy(title_, title);

	if (image_for_title_){

		GL_DeleteTextures(1, &image_for_title_);
	}

	image_for_title_ = Img::makeTextImage(title_, &w_for_title_, &h_for_title_, "ui/default.ttf", 16, 0x0);
}

void Slider::setScriptTarget(const char* str){

	strcpy(script_target_, str);
}

void Slider::addSlider(SliderBody* pframe){

	pslider_body_ = pframe;
	pslider_body_->setPos(getPosX() + 71.0f, getPosY() + 24.0f);

	if (lua_state_){
		if (strlen(script_target_)){

			char script_cmd[256];
			snprintf(script_cmd, sizeof(script_cmd), "%s = %f", script_target_, pslider_body_->getSliderValue());
			luaL_dostring(lua_state_, script_cmd);
		}
	}
	addChild(pslider_body_);
}

int Slider::procMessage(int msg, int param0, int param1){

	if (!pslider_body_)
		return 0;

	if (msg == USER_MSG_UPDATED){

		char script_cmd[256];
		snprintf(script_cmd, sizeof(script_cmd), "%s = %f", script_target_, pslider_body_->getSliderValue());
		luaL_dostring(lua_state_, script_cmd);
	}
	return 0;
}