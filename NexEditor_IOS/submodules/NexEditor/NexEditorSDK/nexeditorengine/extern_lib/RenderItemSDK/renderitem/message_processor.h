#pragma once
#include "frame.h"

//We don't care about other mouse buttons but for left button
//For the simplicity

class MessageProcessor{

	enum{
		MOUSE_STATE_IDLE = 0,
		MOUSE_STATE_BTN_DOWN,
		MOUSE_STATE_BTN_PRESS,
		MOUSE_STATE_BTN_UP,
		MOUSE_STATE_DRAGGING,
	};

	int mouse_state_;
	Frame* pprev_frame_;
	Frame* ptop_frame_;
	bool show_ui_;
public:

	void setTopFrame(Frame* pframe);

	Frame* getTopFrame();

	MessageProcessor();

	~MessageProcessor();

	Frame* findFrame(int x, int y);

	int procMouseButtonDown(SDL_MouseButtonEvent& event);

	int procMouseButtonUp(SDL_MouseButtonEvent& event);

	int procMouseMove(SDL_MouseMotionEvent& event);

	int procMouseWheel(SDL_MouseWheelEvent& event);

	int procDropfile(SDL_DropEvent& event);

	int procKeydown(SDL_KeyboardEvent& event);

	int doMessageProcess(SDL_Event& event);

	void doDraw();

	void updateUI();
};
