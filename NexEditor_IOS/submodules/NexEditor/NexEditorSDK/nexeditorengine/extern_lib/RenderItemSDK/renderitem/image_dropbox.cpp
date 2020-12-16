#include "image_dropbox.h"
#include "glext_loader.h"
#include "lua_extension_for_renderItem.h"
#include "RenderItem.h"

ImageDropbox::ImageDropbox(Frame* parent, const char* filename, const char* target, const char* default_value) :Frame(parent, filename), target_(target){

	image_for_title_.set(target, "ui/default.ttf", 16, 0x01);
	if (default_value){

		RenderItem* prender_item = getRenderItem();
		if (prender_item) prender_item->doListenerWork(target_.c_str(), default_value);
	}
}

ImageDropbox::~ImageDropbox(){


}

void ImageDropbox::draw(){

	Frame::draw();

	glUseProgram(program);
	setMatrix(x_, y_, image_for_title_.w_, image_for_title_.h_);
	setAlpha(1.0f);
	setTexture(image_for_title_.texname_);
	drawRect(image_for_title_.w_, image_for_title_.h_);

	setMatrix(x_, y_ + 16.0f, image_for_value_.w_, image_for_value_.h_);
	setAlpha(1.0f);
	setTexture(image_for_value_.texname_);
	drawRect(image_for_value_.w_, image_for_value_.h_);
}

int ImageDropbox::procMessage(int msg, int param0, int param1){

	if (msg == SDL_DROPFILE){

		SDL_DropEvent& event = *((SDL_DropEvent*)param0);
		image_for_value_.set(event.file, "ui/default.ttf", 10, 0x0);
		if (image_for_value_.w_ > image_for_title_.w_)
			w_ = image_for_value_.w_;
		else
			w_ = image_for_title_.w_;
		RenderItem* prender_item = getRenderItem();
		if (prender_item) 
			prender_item->doListenerWork(target_.c_str(), event.file);
		
		return 0;
	}

	return 1;
}