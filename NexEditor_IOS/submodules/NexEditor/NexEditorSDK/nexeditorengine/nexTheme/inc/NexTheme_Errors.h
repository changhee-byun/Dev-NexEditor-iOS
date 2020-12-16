//
//  NexTheme_Errors.h
//

#ifndef NexTheme_NexTheme_Errors_h
#define NexTheme_NexTheme_Errors_h

typedef enum NXT_Error_ {
	NXT_Error_None                      = 0,
	NXT_Error_Internal                  = 1,
	NXT_Error_Limits                    = 2,
	NXT_Error_Syntax                    = 3,
	NXT_Error_Malloc                    = 4,
	NXT_Error_ParamRange                = 5,
	NXT_Error_MissingParam              = 6,
	NXT_Error_BadParam                  = 7,
	NXT_Error_NoEffect                  = 8,
	NXT_Error_ShaderFailure             = 9,
	NXT_Error_OpenGLFailure             = 10,
	NXT_Error_InvalidState              = 11,
	NXT_Error_GLInitFail                = 12,
	NXT_Error_GLNoConfig                = 13,
	NXT_Error_GLContextErr              = 14,
	NXT_Error_GLSurfaceErr              = 15,
	NXT_Error_WrongRendererType         = 16,
	NXT_Error_NoNativeSurface           = 17,
	NXT_Error_NoContext                 = 18,
	NXT_Error_PThread_General           = 19,
	NXT_Error_PThread_Deadlock          = 20,
	NXT_Error_PThread_Inval             = 21,
	NXT_Error_PThread_Srch              = 22,
	NXT_Error_PThread_BadAttr           = 23,
	NXT_Error_PThread_Perm              = 24,
	NXT_Error_PThread_Resources         = 25,
	NXT_Error_WouldDeadlock             = 26,
	NXT_Error_TooManySurfaceTextures    = 27,
	NXT_Error_WrongOwner                = 28,
	NXT_Error_NotInit                   = 29,
	NXT_Error_JVM                       = 30,
	NXT_Error_ConstructorFail           = 31,
	NXT_Error_NoJVM                     = 32,
	NXT_Error_GLNoDisplay               = 33,
    NXT_Error_InvalidXML                = 34,
    NXT_Error_Timeout                   = 35
} NXT_Error;

#endif
