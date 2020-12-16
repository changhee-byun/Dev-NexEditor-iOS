#pragma once


namespace RenderTarget{
	enum {

		Normal,
		Mask
	};
};


namespace RenderMode{
	enum {

		Preview,
		Export,
	};	
};

#define  CHECK_GL_ERROR()