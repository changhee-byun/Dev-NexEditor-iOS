#pragma once

#include "frame.h"

class RootFrame :public Frame{

public:
	RootFrame() :Frame(NULL, NULL){

		setMovable(false);
	}

	~RootFrame(){


	}

	/*void setPos(float x, float y){

	}*/

	void addPos(float x, float y, Frame* pframe = NULL){

	}
};

