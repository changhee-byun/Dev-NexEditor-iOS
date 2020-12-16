#include "message_processor.h"

Uint32 GetMouseState(int *x, int *y){

#ifdef DEMONSTRATION

	return SDL_GetMouseState(x, y);
#endif
	return 0;
}

Uint32 GetRelativeMouseState(int *x, int *y){

#ifdef DEMONSTRATION

	return SDL_GetRelativeMouseState(x, y);
#endif//DEMONSTRATION
	return 0;
}

void MessageProcessor::setTopFrame(Frame* pframe){

	ptop_frame_ = pframe;
}

Frame* MessageProcessor::getTopFrame(){

	return ptop_frame_;
}

MessageProcessor::MessageProcessor() :pprev_frame_(NULL), ptop_frame_(NULL), mouse_state_(MOUSE_STATE_IDLE), show_ui_(false){

}

MessageProcessor::~MessageProcessor(){


}

Frame* MessageProcessor::findFrame(int x, int y){

	if (!ptop_frame_)
		return NULL;
	return ptop_frame_->doFind(x, y);
}

int MessageProcessor::procMouseButtonDown(SDL_MouseButtonEvent& event){

	if (event.button != SDL_BUTTON_LEFT)
		return 1;

	Frame* pframe = findFrame(event.x, event.y);

	if (pframe){
		ptop_frame_->doSelect(event.x, event.y);
		if (pframe == pprev_frame_){

		}
		mouse_state_ = MOUSE_STATE_BTN_DOWN;
	}
	pprev_frame_ = pframe;

	return 0;
}

int MessageProcessor::procMouseButtonUp(SDL_MouseButtonEvent& event){

	if (event.button != SDL_BUTTON_LEFT)
		return 1;

	Frame* pframe = findFrame(event.x, event.y);

	if (pframe){

		if (pframe == pprev_frame_){

			//Generally we think it as a click event
			//But if there is much time gap or much position move on click, can we think it as a click event???
			//And I will change this ugly ifelse chunks with a nice state pattern
			if (MOUSE_STATE_BTN_DOWN == mouse_state_){

				pframe->procMessage(USER_MSG_CLICK, event.x, event.y);
			}
			else if (MOUSE_STATE_DRAGGING == mouse_state_){


			}
		}
	}
	pprev_frame_ = pframe;
	mouse_state_ = MOUSE_STATE_IDLE;

	return 0;
}

int MessageProcessor::procMouseMove(SDL_MouseMotionEvent& event){

	if (pprev_frame_){

		if (MOUSE_STATE_BTN_DOWN == mouse_state_){

			if (event.state & SDL_BUTTON_LMASK){

				printf("Mouse Move:%d %d\n", event.xrel, event.yrel);
				mouse_state_ = MOUSE_STATE_DRAGGING;
				pprev_frame_->addPos((float)event.xrel, (float)event.yrel);

				int rel_x = 0, rel_y = 0;
				GetRelativeMouseState(&rel_x, &rel_y);

				return 0;
			}
		}
	}

	return 1;
}

int MessageProcessor::procMouseWheel(SDL_MouseWheelEvent& event){

	if (pprev_frame_){

		pprev_frame_->addSize(event.y, event.y);
		return 0;
	}
	return 1;
}

int MessageProcessor::procDropfile(SDL_DropEvent& event){

	int x(0), y(0);

	GetMouseState(&x, &y);
	Frame* pframe = findFrame(x, y);
	if (pframe){

		pframe->procMessage(SDL_DROPFILE, (int)&event, 0);
		return 0;
	}

	return 1;
}

int MessageProcessor::procKeydown(SDL_KeyboardEvent& event){

	if (!ptop_frame_)
		return 0;
	if (event.keysym.scancode == SDL_SCANCODE_F12){

		show_ui_ = !show_ui_;
		updateUI();
	}

	return 0;
}

int MessageProcessor::doMessageProcess(SDL_Event& event){

	if (MOUSE_STATE_DRAGGING == mouse_state_ && pprev_frame_){

		int rel_x = 0, rel_y = 0;

		GetRelativeMouseState(&rel_x, &rel_y);
		pprev_frame_->addPos((float)rel_x, (float)rel_y);
	}
	switch (event.type)
	{

	case SDL_KEYDOWN:
		return procKeydown(event.key);

	case SDL_MOUSEBUTTONDOWN:
		return procMouseButtonDown(event.button);

	case SDL_MOUSEBUTTONUP:
		return procMouseButtonUp(event.button);

	case SDL_MOUSEMOTION:
		return procMouseMove(event.motion);

	case SDL_MOUSEWHEEL:
		return procMouseWheel(event.wheel);

	case SDL_DROPFILE:
		return procDropfile(event.drop);
	};

	return 1;
}

void MessageProcessor::doDraw(){

	if (!ptop_frame_)
		return;
	ptop_frame_->doDraw();
}

void MessageProcessor::updateUI(){

	if (show_ui_){

		ptop_frame_->setPos(0.0f, 0.0f);
	}
	else{

		ptop_frame_->setPos(-1000.0f, 0.0f);
	}
}