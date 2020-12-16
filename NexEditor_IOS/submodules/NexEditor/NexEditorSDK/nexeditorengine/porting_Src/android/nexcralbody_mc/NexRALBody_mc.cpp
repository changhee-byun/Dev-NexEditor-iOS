/******************************************************************************
* File Name        : NexRALBody_mc.cpp
* Description      : Generalized native video renderer for Android
*******************************************************************************
* Copyright (c) 2002-2018 NexStreaming Corp. All rights reserved.
* http://www.nexstreaming.com
*
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
* PURPOSE.
******************************************************************************/

#define LOCATION_TAG "NexRALBody_mc"


#undef LOG_TAG
#define LOG_TAG "nexcral_mc"

#include <stdlib.h>

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include "./NexRALBody_mc.h"
#include "./NexCRALBody_mc_impl.h"

#include "external/NexMediaCodec.h"
#include "external/jni/jni_SurfaceTexture.h"
#include "external/recvd/recvd_SurfaceTexture.h"

using Nex_MC::Common::NexCAL_mc;

#include "version.h"

#include "utils/utils.h"
#include "utils/jni.h"

#include "NexMediaDef.h" //#include "nexOTIs.h"

#if defined(_SHOW_LOGO_)
#include "nexstreaminglogo.h"
#endif

#include "mysystem/window.h"
#include <android/native_window.h>
#define NEX_MC_TYPE NEX_MC_TYPE_RENDERER
#define NEX_MC_AVTYPE NEX_MC_AVTYPE_VIDEO

//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// original in nexRALBody_Common_API.h
typedef void (*FNRALCALLBACK)(int msg, int ext1, int ext2, int ext3, void *ext4, void *userData);
//------------------------------------------------------------------------------
// original in nexRALBody_Video_API.h
typedef	unsigned int (*NEXRALBody_Video_getProperty)(unsigned int uiProperty, unsigned int* puValue, void* uUserData);
typedef unsigned int (*NEXRALBody_Video_setProperty)(unsigned int uProperty, unsigned int uValue, void* uUserData);

typedef unsigned int (*NEXRALBody_Video_init)(NEX_CODEC_TYPE eCodecType, unsigned int uWidth, unsigned int uHeight, unsigned int uPitch, void** puUserData);
typedef unsigned int (*NEXRALBody_Video_deinit)(void* uUserData);
typedef unsigned int (*NEXRALBody_Video_display)(int bDisplay, unsigned int uCTS, unsigned char* pBits1, unsigned char* pBits2, unsigned char* pBits3,  unsigned int *pResult, void* uUserData);

typedef unsigned int ( *NEXRALBody_Video_pause ) ( void* uUserData );
typedef unsigned int ( *NEXRALBody_Video_resume ) ( void* uUserData );
typedef unsigned int ( *NEXRALBody_Video_flush ) ( unsigned int uCTS, void* uUserData );

typedef unsigned int (*NEXRALBody_Video_create)(int nLogLevel, FNRALCALLBACK fnCallback, void *pUserData,
												unsigned int uiLogo, int nLogoPos, int nLogoTime, int nLogoPeriod, unsigned int uiCpuInfo,
												unsigned int uiPlatformInfo, unsigned int uiRenderInfo, unsigned int uiDeviceColorFormat, unsigned int uiIOMXColorFormat);
typedef unsigned int (*NEXRALBody_Video_delete)(void *pUserData);

typedef void* (*NEXRALBody_Video_GetFrameBuffer)(unsigned int uWidth, unsigned int uHeight, unsigned int uPixelFormat);
typedef unsigned int (*NEXRALBody_Video_prepareSurface)(void* pSurface, void* uUserData);
typedef unsigned int (*NEXRALBody_Video_setRenderOption)(unsigned int uiFlag, void* uUserData);
typedef unsigned int (*NEXRALBody_Video_setOutputPos)(int iX, int iY, unsigned int uiWidth, unsigned int uiHeight, void* uUserData);
typedef unsigned int (*NEXRALBody_Video_OnOff)(unsigned int bOn, unsigned int bErase, void* uUserData);
typedef unsigned int (*NEXRALBody_Video_Capture)(unsigned int uiCount, unsigned int uiInterval, void* uUserData);

typedef unsigned int (*NEXRALBody_Video_SetBitmap)(void* jniEnv, void* Bitmap, void* uUserData);
typedef unsigned int (*NEXRALBody_Video_FillBitmap)(void* jniEnv, void* Bitmap, void* uUserData);
typedef unsigned int (*NEXRALBody_Video_GLInit)(int width, int height, void* uUserData);
typedef unsigned int (*NEXRALBody_Video_GLDraw)(int nMode, void* uUserData);
typedef unsigned int (*NEXRALBody_Video_SetContrastBrightness)(int contrast, int brightness, void* uUserData);
typedef unsigned int (*NEXRALBody_Video_SetGLDisplayStop)(unsigned int isStopped, void* uUserData);

typedef int (*NEXRALBody_Video_DisplayBlank)( void *userData );

typedef struct _VIDEO_RALBODY_FUNCTION_
{

	unsigned int								uiCPUInfo;
	unsigned int								uiPlatformInfo;
	unsigned int								uiRenderInfo;
	unsigned int								uiDevColorFormat;
	unsigned int								uiIOMXColorFormat;

	NEXRALBody_Video_getProperty 				fnNexRALBody_Video_getProperty;
	NEXRALBody_Video_setProperty				fnNexRALBody_Video_setProperty;
	NEXRALBody_Video_init						fnNexRALBody_Video_init;
	NEXRALBody_Video_deinit					fnNexRALBody_Video_deinit;
	NEXRALBody_Video_pause					fnNexRALBody_Video_pause;
	NEXRALBody_Video_resume					fnNexRALBody_Video_resume;
	NEXRALBody_Video_flush					fnNexRALBody_Video_flush;
	NEXRALBody_Video_display					fnNexRALBody_Video_display;
	NEXRALBody_Video_create					fnNexRALBody_Video_create;
	NEXRALBody_Video_delete					fnNexRALBody_Video_delete;
	NEXRALBody_Video_prepareSurface			fnNexRALBody_Video_prepareSurface;
	NEXRALBody_Video_setRenderOption			fnNexRALBody_Video_setRenderOption;
	NEXRALBody_Video_setOutputPos				fnNexRALBody_Video_setOutputPos;
	NEXRALBody_Video_OnOff					fnNexRALBody_Video_OnOff;
	NEXRALBody_Video_Capture					fnNexRALBody_Video_Capture;
	NEXRALBody_Video_SetBitmap				fnNexRALBody_Video_SetBitmap;
	NEXRALBody_Video_FillBitmap				fnNexRALBody_Video_FillBitmap;
	NEXRALBody_Video_GLInit					fnNexRALBody_Video_GLInit;
	NEXRALBody_Video_GLDraw					fnNexRALBody_Video_GLDraw;
	NEXRALBody_Video_SetContrastBrightness		fnNexRALBody_Video_SetContrastBrightness;
	NEXRALBody_Video_DisplayBlank			fnNexRALBody_Video_DisplayBlank; //for ics native window.
	NEXRALBody_Video_DisplayBlank			fnNexRALBody_Video_DisplayLogo; //for ics native window.
	NEXRALBody_Video_SetGLDisplayStop		fnNexRALBody_Video_SetGLDisplayStop;
}VIDEO_RALBODY_FUNCTION_ST;

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

#include <list>
namespace { // (anon2)
std::list<Nex_MC::Video::Renderer::NexRAL_mcvr *> instanceList;
pthread_mutex_t instanceListMutex;

class InstanceListInit {
public:
	InstanceListInit()
	{
		//STATUS(Log::MUTEX, Log::DEBUG, "O(%s)", "instanceListMutex");
		pthread_mutex_init(&instanceListMutex, NULL);
	}

	~InstanceListInit()
	{
		//STATUS(Log::MUTEX, Log::DEBUG, "X(%s)", "instanceListMutex");
		pthread_mutex_destroy(&instanceListMutex);
	}
}; // class InstanceListInit
InstanceListInit instanceListInit;
}; // namespace (anon2)

#define MAX_TEXTURES_NUMBER 4
namespace { // (anon3)

	const char gVertexShader[] =
			"precision highp float;\n"
			"attribute vec4 aPosition;\n"
			"attribute vec2 aTextureCoordinates;\n"
			"varying vec2 vTextureCoordinates;\n"
			"void main() {\n"
			"  gl_Position = aPosition;\n"
			"  vTextureCoordinates = aTextureCoordinates;\n"
			"}\n";

	const char gFragmentShader[] =
			"#extension GL_OES_EGL_image_external : require\n"
			"precision highp float;\n"
			"uniform samplerExternalOES uTexture;\n"
			"varying vec2 vTextureCoordinates;\n"
			"void main() {\n"
			"  gl_FragColor = texture2D(uTexture, vTextureCoordinates);\n"
			"}\n";

	const char gStdVertexShader_Textured[] =
			"attribute vec2 a_position;\n"
			"attribute vec2 a_texCoord;\n"
			"uniform mat4 u_mvp_matrix;\n"
			"uniform mat4 u_tex_matrix;\n"
			"varying highp vec2 v_texCoord;\n"
			"void main() {\n"
			"  v_texCoord = (u_tex_matrix * vec4(a_texCoord,0.0,1.0)).st;\n"
			"  gl_Position = (u_mvp_matrix * vec4(a_position, 0.0, 1.0));\n"
			"}\n";

	const char gFragmentShader_TexturedExternal[] =
			"#extension GL_OES_EGL_image_external : require\n"
			"varying highp vec2 v_texCoord;\n"
			"uniform samplerExternalOES u_textureSampler;\n"
			"uniform highp float u_alpha;\n"
			"void main() {\n"
			"  highp vec4 color = texture2D(u_textureSampler, v_texCoord);\n"
			"  highp float f = step(0.0,v_texCoord.y) * step(0.0,v_texCoord.x) * step(v_texCoord.y,1.0) * step(v_texCoord.x,1.0);\n"
			"  gl_FragColor = u_alpha * color * vec4(color.a*f, color.a*f, color.a*f, 1.0);\n"
			"}\n";

	const GLfloat FULL_SCREEN_SURFACE_VERTICES[] = {
			-1.0f, 1.0f,
			-1.0f, -1.0f,
			1.0f, 1.0f,
			1.0f, -1.0f };

	const GLfloat FULL_SURFACE_TEXTURE_VERTICES[] = {
			0.0f, 1.0f,
			0.0f, 0.0f,
			1.0f, 1.0f,
			1.0f, 0.0f };

	const GLfloat IDENTITY_MATRIX[] = {
            1.0, 0.0, 0.0, 0.0,
            0.0, 1.0, 0.0, 0.0,
            0.0, 0.0, 1.0, 0.0,
            0.0, 0.0, 0.0, 1.0 };

	void printGLString(const char *name, GLenum s)
	{
		const char *v = (const char *) glGetString(s);
		STATUS(Nex_MC::Log::OUTPUT, Nex_MC::Log::DEBUG, "GL %s = %s", name, v);
	}

	void checkGlError(const char* op)
	{
		for (GLint error = glGetError(); error; error = glGetError()) {
			LOGE("after %s() glError (0x%x)\n", op, error);
		}
	}

	GLuint loadShader(GLenum shaderType, const char* pSource)
	{
		GLuint shader = glCreateShader(shaderType);
		if (shader)
		{
			glShaderSource(shader, 1, &pSource, NULL);
			glCompileShader(shader);
			GLint compiled = 0;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
			if (!compiled)
			{
				GLint infoLen = 0;
				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
				if (infoLen)
				{
					char* buf = (char*) malloc(infoLen);
					if (buf)
					{
						glGetShaderInfoLog(shader, infoLen, NULL, buf);
						LOGE("Could not compile shader %d:\n%s\n", shaderType, buf);
						free(buf);
					}
					glDeleteShader(shader);
					shader = 0;
				}
			}
		}
		return shader;
	}

	GLuint createProgram(const char* pVertexSource, const char* pFragmentSource)
	{
		GLuint vertexShader = loadShader(GL_VERTEX_SHADER, pVertexSource);
		if (!vertexShader)
		{
			return 0;
		}

		GLuint pixelShader = loadShader(GL_FRAGMENT_SHADER, pFragmentSource);
		if (!pixelShader)
		{
			return 0;
		}

		GLuint program = glCreateProgram();
		if (program)
		{
			glAttachShader(program, vertexShader);
			checkGlError("glAttachShader");

			glAttachShader(program, pixelShader);
			checkGlError("glAttachShader");

			glLinkProgram(program);

			GLint linkStatus = GL_FALSE;

			glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);

			if (linkStatus != GL_TRUE)
			{
				GLint bufLength = 0;
				glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);

				if (bufLength)
				{
					char* buf = (char*) malloc(bufLength);
					if (buf)
					{
						glGetProgramInfoLog(program, bufLength, NULL, buf);
						LOGE("Could not link program:\n%s\n", buf);
						free(buf);
					}
				}
				glDeleteProgram(program);
				program = 0;
			}
		}

		if (0 != vertexShader)
		{
			glDeleteShader(vertexShader);
		}

		if (0 != pixelShader)
		{
			glDeleteShader(pixelShader);
		}

		return program;
	}

#if defined(_SHOW_LOGO_)
	const char gLogoFragmentShader_Textured[] =
			"varying highp vec2 v_texCoord;\n"
			"uniform sampler2D u_textureSampler;\n"
			"uniform highp float u_alpha;\n"
			"void main() {\n"
			"  highp vec4 color = texture2D(u_textureSampler, v_texCoord);\n"
			"  highp float f = step(0.0,v_texCoord.y) * step(0.0,v_texCoord.x) * step(v_texCoord.y,1.0) * step(v_texCoord.x,1.0);\n"
			"  gl_FragColor = u_alpha * color * vec4(color.a*f, color.a*f, color.a*f, 1.0);\n"
			"}\n";
#endif

	GLuint g_texturesQueuedForDeletion[128] = { 0, };
	EGLContext g_eglContextOfTexturesQueuedForDeletion[128] = { 0, };
	size_t g_numTexturesQueuedForDeletion = 0;

	pthread_mutex_t textureDeletionMutex;

	void queueTextureForDeletion(GLuint texName, EGLContext context)
	{
		pthread_mutex_lock(&textureDeletionMutex);

		bool added = false;

		for (size_t i = 0; i < 128; ++i)
		{
			if (0 == g_texturesQueuedForDeletion[i])
			{
				g_texturesQueuedForDeletion[i] = texName;
				g_eglContextOfTexturesQueuedForDeletion[i] = context;
				++g_numTexturesQueuedForDeletion;
				added = true;
				break;
			}
		}

		if (!added)
		{
			MC_WARN("could not add texture to queue for future deletion");
		}

		pthread_mutex_unlock(&textureDeletionMutex);
	}

	void deleteQueuedTextures()
	{
		EGLContext currentContext = eglGetCurrentContext();

		pthread_mutex_lock(&textureDeletionMutex);

		size_t numDeleted = 0;

		for (size_t i = 0, numFound = 0; i < 128 && numFound < g_numTexturesQueuedForDeletion; ++i)
		{
			if (0 != g_texturesQueuedForDeletion[i])
			{
				++numFound;
				if (g_eglContextOfTexturesQueuedForDeletion[i] == currentContext)
				{
					glDeleteTextures(1, &g_texturesQueuedForDeletion[i]);
					g_texturesQueuedForDeletion[i] = 0;
					g_eglContextOfTexturesQueuedForDeletion[i] = EGL_NO_CONTEXT;
					++numDeleted;
				}
			}
		}

		g_numTexturesQueuedForDeletion -= numDeleted;

		pthread_mutex_unlock(&textureDeletionMutex);
	}

	class TextureDeletionQueueInit {
	public:
		TextureDeletionQueueInit()
		{
			//STATUS(Log::MUTEX, Log::DEBUG, "O(%s)", "textureDeletionMutex");
			pthread_mutex_init(&textureDeletionMutex, NULL);
		}

		~TextureDeletionQueueInit()
		{
			//STATUS(Log::MUTEX, Log::DEBUG, "X(%s)", "textureDeletionMutex");
			pthread_mutex_destroy(&textureDeletionMutex);
		}
	}; // class TextureDeletionQueueInit
	TextureDeletionQueueInit textureDeletionQueueInit;
}; // namespace (anon3)

namespace Nex_MC {

namespace Video {

namespace Renderer {

unsigned int GetProperty( unsigned int uProperty, unsigned int *puValue, void* uUserData )
{
	STATUS(Log::FLOW, Log::INFO, "+ prop(0x%X) ud(0x%p)", uProperty, uUserData);

	unsigned int retValue = 0;

	unsigned int input = *puValue;

	switch ( uProperty )
	{
	default:
		*puValue = 0;
		break;
	}

	STATUS(Log::FLOW, Log::INFO, "- ret(0x%X) value(0x%X)", retValue, *puValue);
	return retValue;
}

unsigned int SetProperty( unsigned int uProperty, unsigned int uValue, void* uUserData )
{
	STATUS(Log::FLOW, Log::INFO, "+ prop(0x%X) value(0x%X) ud(0x%p)", uProperty, uValue, uUserData);

	unsigned int retValue = 0;

	STATUS(Log::FLOW, Log::INFO, "- ret(0x%X)", retValue);
	return retValue;
}

unsigned int Init
	( NEX_CODEC_TYPE eCodecType
	, unsigned int uWidth
	, unsigned int uHeight
	, unsigned int uPitch
	, void **puUserData )
{
	IDENTITY("+", "VR", *puUserData, "");

	unsigned int retValue = -1;

	STATUS(Log::MUTEX, Log::DEBUG, "++(%s)", "instanceListMutex");
	pthread_mutex_lock(&instanceListMutex);
	STATUS(Log::MUTEX, Log::DEBUG, "(%s)++", "instanceListMutex");

	for (std::list<NexRAL_mcvr *>::iterator it = instanceList.begin(); it != instanceList.end(); ++it)
	{
		if (*puUserData == (*it)->hPlayer)
		{
			*puUserData = (*it);

			if (NULL != (*it)->ralCallbackFunction)
			{
				(*it)->ralCallbackFunction(0x70001, uWidth, uHeight, 0, NULL, (*it)->ralCallbackUserData);
			}

			retValue = 0;
			break;
		}
	}

	STATUS(Log::MUTEX, Log::DEBUG, "--(%s)", "instanceListMutex");
	pthread_mutex_unlock(&instanceListMutex);

	if (0 != retValue)
	{
		LOGE("Couldn't find matching initialized ral instance. Make sure you call Create first");
	}
	else
	{
		NexRAL_mcvr *mcvr = (NexRAL_mcvr *)(*puUserData);
		mcvr->hasNoFrame = true;
	}

	IDENTITY("-", "VR", *puUserData, "ret(0x%X)", retValue);
	return retValue;
}

unsigned int Deinit( void* uUserData )
{
	IDENTITY("+", "VR", uUserData, "");

	if (NULL != (void *)uUserData)
	{
		NexRAL_mcvr *mcvr = (NexRAL_mcvr *)uUserData;

		mcvr->hasNoFrame = true;

		if (NULL != mcvr->ralCallbackFunction)
		{
			mcvr->ralCallbackFunction(0x70002, 0, 0, 0, NULL, mcvr->ralCallbackUserData);
		}
	}

	IDENTITY("-", "VR", uUserData, "ret(0x%X)", 0);
	return 0;
}

unsigned int Pause(void* uUserData)
{
	IDENTITY("+-", "VR", uUserData, "ret(0x%X)", 0); // +- will mean nothing done
	return 0;
}

unsigned int Resume(void* uUserData)
{
	IDENTITY("+-", "VR", uUserData, "ret(0x%X)", 0); // +- will mean nothing done
	return 0;
}

unsigned int Flush(unsigned int uCTS, void* uUserData)
{
	IDENTITY("+-", "VR", uUserData, "ret(0x%X)", 0); // +- will mean nothing done
	return 0;
}

unsigned int Display( 
		int bDisplay
		, unsigned int uPTS
		, unsigned char *pBits1
		, unsigned char *pBits2
		, unsigned char *pBits3
		, unsigned int* pResult
		, void* uUserData )
{
	IDENTITY("+", "VR", uUserData, "display(%s) pts(%u)", FALSE != bDisplay ? "true" : "false", uPTS);

	if (NULL == (void *)uUserData)
	{
		MC_ERR("invalid userdata received");
		*pResult = 0;
		IDENTITY("-", "VR", uUserData, "ret(0x%X) pResult(0x%X)", -1, *pResult);
		return -1;
	}

	NexRAL_mcvr *mcvr = (NexRAL_mcvr *)uUserData;

	unsigned int retValue = 0;

	if (!mcvr->on)
	{
		bDisplay = FALSE;
	}

	long outIndex = (long)pBits1;

	if (pBits1 == pBits2 && pBits2 == pBits3)
	{
		STATUS(Log::EXTERNAL_CALL, Log::DEBUG, "->releaseOutputBuffer");
		mcvr->mc->vd.releaseOutputBuffer(mcvr->mc, outIndex, FALSE != bDisplay);
		STATUS(Log::EXTERNAL_CALL, Log::DEBUG, "releaseOutputBuffer->");

		mcvr->hasNoFrame = false;
	}
	else
	{
		bDisplay = FALSE;
	}

	if (FALSE != bDisplay)
	{
		if (NULL != mcvr->ralCallbackFunction)
		{
			pthread_mutex_lock(&mcvr->glDrawStatusLock);
			mcvr->glDrawWaiting = true;
			pthread_mutex_unlock(&mcvr->glDrawStatusLock);
			mcvr->ralCallbackFunction( 0x70003, mcvr->mc->vd.width, mcvr->mc->vd.height, 0, NULL, mcvr->ralCallbackUserData );
			pthread_mutex_lock(&mcvr->glDrawStatusLock);
			if (mcvr->glDrawWaiting)
			{
				int timeoutInMillis = 15;
				struct timespec timeout;
				struct timeval now;
				gettimeofday(&now, NULL);
				timeout.tv_nsec = (now.tv_usec + ((timeoutInMillis % 1000) * 1000)) * 1000;
				timeout.tv_sec = now.tv_sec + timeoutInMillis / 1000;
				while (1000000000 < timeout.tv_nsec)
				{
					timeout.tv_nsec -= 1000000000;
					++timeout.tv_sec;
				}
				pthread_cond_timedwait(&mcvr->glDrawStatusCondition, &mcvr->glDrawStatusLock, &timeout);
			}
			pthread_mutex_unlock(&mcvr->glDrawStatusLock);
		}
	}

	*pResult = (FALSE != bDisplay ? NEXRAL_VIDEO_RET_DISPLAY_SUCCESS : 0) | NEXRAL_VIDEO_RET_DISPLAY_NEXT_FRAME;

	IDENTITY("-", "VR", uUserData, "ret(0x%X) pResult(0x%X)", retValue, *pResult);
	return retValue;
}

unsigned int surfaceDisplay( 
		int bDisplay
		, unsigned int uPTS
		, unsigned char *pBits1
		, unsigned char *pBits2
		, unsigned char *pBits3
		, unsigned int* pResult
		, void* uUserData )
{
	IDENTITY("+", "VR", uUserData, "display(%s) pts(%u)", FALSE != bDisplay ? "true" : "false", uPTS);

	if (NULL == (void *)uUserData)
	{
		MC_ERR("invalid userdata received");
		*pResult = 0;
		IDENTITY("-", "VR", uUserData, "ret(0x%X) pResult(0x%X)", -1, *pResult);
		return -1;
	}

	NexRAL_mcvr *mcvr = (NexRAL_mcvr *)uUserData;

	unsigned int retValue = 0;

	if (!mcvr->on)
	{
		bDisplay = FALSE;
	}

	long outIndex = (long)pBits1;

	if (pBits1 == pBits2 && pBits2 == pBits3)
	{
		STATUS(Log::EXTERNAL_CALL, Log::DEBUG, "->releaseOutputBuffer");
		mcvr->mc->vd.releaseOutputBuffer(mcvr->mc, outIndex, FALSE != bDisplay);
		STATUS(Log::EXTERNAL_CALL, Log::DEBUG, "releaseOutputBuffer->");

		mcvr->hasNoFrame = false;
	}
	else
	{
		bDisplay = FALSE;
	}

	if (FALSE != bDisplay)
	{
		if (NULL != mcvr->ralCallbackFunction)
		{
			mcvr->ralCallbackFunction( 0x70003, mcvr->mc->vd.width, mcvr->mc->vd.height, 0, NULL, mcvr->ralCallbackUserData );
		}
	}

	*pResult = (FALSE != bDisplay ? NEXRAL_VIDEO_RET_DISPLAY_SUCCESS : 0) | NEXRAL_VIDEO_RET_DISPLAY_NEXT_FRAME;

	IDENTITY("-", "VR", uUserData, "ret(0x%X) pResult(0x%X)", retValue, *pResult);
	return retValue;
}

unsigned int prepareSurface(void* pSurface, void* uUserData)
{
	IDENTITY("+", "VR", uUserData, "pSurface(%p) ", pSurface);

	if (NULL == (void *)uUserData)
	{
		MC_ERR("invalid userdata received");
		IDENTITY("-", "VR", uUserData, "ret(0x%X)", -1);
		return -1;
	}

	NexRAL_mcvr *mcvr = (NexRAL_mcvr *)uUserData;

	jobject surface = NULL;
	if (NULL == pSurface)
	{
		MC_ERR("invalid pSurface received");
		IDENTITY("-", "VR", uUserData, "ret(0x%X)", -1);
		return -1;
	}

	surface = (jobject)pSurface;

	Nex_MC::Utils::JNI::JNIEnvWrapper env;
	if (env != NULL)
	{
		jobject oldSurface = surface;
		surface = env->NewGlobalRef(oldSurface);
	}

	NexSurfaceTexture_using_recvd::createSurfaceTexture(surface, mcvr->surfaceTexture);

	IDENTITY("-", "VR", uUserData, "ret(0x%X)", 0);
	return 0;
}

unsigned int Create(int nLogLevel, FNRALCALLBACK fnCallback, void *pUserData,
	unsigned int uiLogo, int nLogoPos, int nLogoTime, int nLogoPeriod, unsigned int uiCpuInfo,
	unsigned int uiPlatformInfo, unsigned int uiRenderInfo, unsigned int uiDeviceColorFormat, unsigned int uiIOMXColorFormat)
{
	Log::SetDebugLevel(NEX_MC_TYPE, NEX_MC_AVTYPE, 0xffffffff, nLogLevel); // maybe make an api for this instead of doing this here every time?
	IDENTITY("+", "VR", *((unsigned int *)pUserData), "");

	void* hPlayer = *((void**)pUserData);

	NexRAL_mcvr *mcvr = (NexRAL_mcvr *)malloc(sizeof(NexRAL_mcvr));
	if (NULL == mcvr)
	{
		MC_ERR("malloc failed!");
		return -1;
	}
	STATUS(Log::FLOW, Log::INFO, "%p = malloc(%d)", mcvr, sizeof(NexRAL_mcvr));
	memset(mcvr, 0x00, sizeof(NexRAL_mcvr));

	mcvr->hasNoFrame = true;
	mcvr->hPlayer = hPlayer;

	mcvr->ralCallbackFunction = (ralCallback_t)fnCallback;
	mcvr->ralCallbackUserData = (void *)hPlayer;

	mcvr->on = true;

	pthread_mutex_init(&mcvr->glDrawStatusLock, NULL);
	pthread_cond_init(&mcvr->glDrawStatusCondition, NULL);

	mcvr->glDrawWaiting = false;

	*((void**)pUserData) = (void*)mcvr;

	STATUS(Log::MUTEX, Log::DEBUG, "++(%s)", "instanceListMutex");
	pthread_mutex_lock(&instanceListMutex);
	STATUS(Log::MUTEX, Log::DEBUG, "(%s)++", "instanceListMutex");

	instanceList.push_front(mcvr); // push_front is used because init will traverse from begin()... the last created is likely the one that wants to be found

	STATUS(Log::MUTEX, Log::DEBUG, "--(%s)", "instanceListMutex");
	pthread_mutex_unlock(&instanceListMutex);

	if (NULL != mcvr->ralCallbackFunction)
	{
		STATUS(Log::EXTERNAL_CALL, Log::DEBUG, "->callback");
		mcvr->ralCallbackFunction(0x70005, 0, 0, 0, 0, mcvr->ralCallbackUserData);
		STATUS(Log::EXTERNAL_CALL, Log::DEBUG, "callback->");
	}

	IDENTITY("-", "VR", *((unsigned int *)pUserData), "ret(0x%X)", 0);
	return 0;
}

unsigned int Delete(void* pUserData)
{
	IDENTITY("+", "VR", pUserData, "");

	if (NULL == pUserData)
	{
		MC_ERR("invalid userdata received");
		IDENTITY("-", "VR", pUserData, "ret(0x%X)", -1);
		return -1;
	}

	NexRAL_mcvr *mcvr = (NexRAL_mcvr *)pUserData;
	if (NULL != mcvr->mc)
	{
		mcvr->mc->vd.mcvr = NULL;
	}

	STATUS(Log::MUTEX, Log::DEBUG, "++(%s)", "instanceListMutex");
	pthread_mutex_lock(&instanceListMutex);
	STATUS(Log::MUTEX, Log::DEBUG, "(%s)++", "instanceListMutex");

	instanceList.remove(mcvr);

	STATUS(Log::MUTEX, Log::DEBUG, "--(%s)", "instanceListMutex");
	pthread_mutex_unlock(&instanceListMutex);

	if (NULL != mcvr->surfaceTexture)
	{
		STATUS(Log::EXTERNAL_CALL, Log::DEBUG, "->releaseNexSurfaceTexture");
		NexSurfaceTexture_using_jni::releaseNexSurfaceTexture(mcvr->surfaceTexture);
		STATUS(Log::EXTERNAL_CALL, Log::DEBUG, "releaseNexSurfaceTexture->");
		mcvr->surfaceTexture = NULL;
	}

	if (0 != mcvr->texName)
	{
		queueTextureForDeletion(mcvr->texName, mcvr->eglcontext);
#if defined(_SHOW_LOGO_)
		//queueTextureForDeletion(mcvr->logoTexName, mcvr->eglcontext);
#endif
	}

	pthread_mutex_destroy(&mcvr->glDrawStatusLock);
	pthread_cond_destroy(&mcvr->glDrawStatusCondition);

	free(mcvr);

	IDENTITY("-", "VR", pUserData, "ret(0x%X)", 0);
	return 0;
}

unsigned int surfaceDelete(void* pUserData)
{
	IDENTITY("+", "VR", pUserData, "");

	if (NULL == pUserData)
	{
		MC_ERR("invalid userdata received");
		IDENTITY("-", "VR", pUserData, "ret(0x%X)", -1);
		return -1;
	}

	NexRAL_mcvr *mcvr = (NexRAL_mcvr *)pUserData;
	if (NULL != mcvr->mc)
	{
		mcvr->mc->vd.mcvr = NULL;
	}

	STATUS(Log::MUTEX, Log::DEBUG, "++(%s)", "instanceListMutex");
	pthread_mutex_lock(&instanceListMutex);
	STATUS(Log::MUTEX, Log::DEBUG, "(%s)++", "instanceListMutex");

	instanceList.remove(mcvr);

	STATUS(Log::MUTEX, Log::DEBUG, "--(%s)", "instanceListMutex");
	pthread_mutex_unlock(&instanceListMutex);

	if (NULL != mcvr->surfaceTexture)
	{
		STATUS(Log::EXTERNAL_CALL, Log::DEBUG, "->releaseNexSurfaceTexture");
		NexSurfaceTexture_using_recvd::releaseNexSurfaceTexture(mcvr->surfaceTexture, false, true);
		STATUS(Log::EXTERNAL_CALL, Log::DEBUG, "releaseNexSurfaceTexture->");
		mcvr->surfaceTexture = NULL;
	}

	pthread_mutex_destroy(&mcvr->glDrawStatusLock);
	pthread_cond_destroy(&mcvr->glDrawStatusCondition);

	free(mcvr);

	IDENTITY("-", "VR", pUserData, "ret(0x%X)", 0);
	return 0;
}

unsigned int SetOutputPos(int iX, int iY, unsigned int uiWidth, unsigned int uiHeight, void* uUserData)
{
	IDENTITY("+", "VR", uUserData, "x,y(%d,%d) WxH(%ux%u)", iX, iY, uiWidth, uiHeight);

	if (NULL == (void *)uUserData)
	{
		MC_ERR("invalid userdata received");
		IDENTITY("-", "VR", uUserData, "ret(0x%X)", -1);
		return -1;
	}

	NexRAL_mcvr *mcvr = (NexRAL_mcvr *)uUserData;

	float translateX = 2*iX / (float)mcvr->surfaceWidth;
	float translateY = 2*iY / (float)mcvr->surfaceHeight;
	float scaleX = uiWidth / (float)mcvr->surfaceWidth;
	float scaleY = uiHeight / (float)mcvr->surfaceHeight;

	STATUS(Log::OUTPUT, Log::VERBOSE, "tX,Y(%f,%f) sX,Y(%f,%f)", translateX, translateY, scaleX, scaleY);

	mcvr->mvp[0] = scaleX;
	mcvr->mvp[5] = scaleY;
	mcvr->mvp[10] = 1;
	mcvr->mvp[12] = scaleX + translateX - 1;
	mcvr->mvp[13] = 1 - scaleY - translateY;
	mcvr->mvp[15] = 1;

	IDENTITY("-", "VR", uUserData, "ret(0x%X)", 0);
	return 0;
}

unsigned int OnOff(unsigned int bOn, unsigned int bErase, void* uUserData)
{
	IDENTITY("+", "VR", uUserData, "on/off(%s) erase(%s)", 0 != bOn ? "on" : "off", 0 != bErase ? "true" : "false");

	if (NULL == (void *)uUserData)
	{
		MC_ERR("invalid userdata received");
		IDENTITY("-", "VR", uUserData, "ret(0x%X)", -1);
		return -1;
	}

	NexRAL_mcvr *mcvr = (NexRAL_mcvr *)uUserData;

	mcvr->on = 0 != bOn;
	if (0 != bErase)
	{
		MC_WARN("erase not implemented");
	}

	IDENTITY("-", "VR", 0, "ret(0x%X)", 0);
	return 0;
}

unsigned int InitOpenGL(int width, int height, void* uUserData)
{
	IDENTITY("+", "VR", uUserData, "WxH(%dx%d)", width, height);

	deleteQueuedTextures();

	if (NULL == (void *)uUserData)
	{
		MC_ERR("invalid userdata received");
		IDENTITY("-", "VR", uUserData, "ret(0x%X)", -1);
		return -1;
	}

	NexRAL_mcvr *mcvr = (NexRAL_mcvr *)uUserData;

	if (0 < width)
		mcvr->surfaceWidth = width;
	if (0 < height)
		mcvr->surfaceHeight = height;

	int retValue = 0;

	if (0 < width && 0 < height)
	{
		printGLString("Version", GL_VERSION);

		mcvr->program = createProgram(gStdVertexShader_Textured, gFragmentShader_TexturedExternal);

		if (!mcvr->program) {
			LOGE("Could not create program.");
			IDENTITY("-", "VR", 0, "ret(0x%X)", -1);
			return -1;
		}

		mcvr->u_mvp_matrix = glGetUniformLocation(mcvr->program, "u_mvp_matrix");
		checkGlError("glGetUniformLocation u_mvp_matrix");
		mcvr->u_tex_matrix = glGetUniformLocation(mcvr->program, "u_tex_matrix");
		checkGlError("glGetUniformLocation u_tex_matrix");
		mcvr->u_textureSampler = glGetUniformLocation(mcvr->program, "u_textureSampler");
		checkGlError("glGetUniformLocation u_textureSampler");
		mcvr->u_alpha = glGetUniformLocation(mcvr->program, "u_alpha");
		checkGlError("glGetUniformLocation u_alpha");

		mcvr->a_position = glGetAttribLocation(mcvr->program, "a_position");
		checkGlError("glGetAttribLocation a_position");
		mcvr->a_texCoord = glGetAttribLocation(mcvr->program, "a_texCoord");
		checkGlError("glGetAttribLocation a_texCoord");

		glViewport(0, 0, width, height);
		checkGlError("glViewport");

#if defined(_SHOW_LOGO_)
		mcvr->logoProgram = createProgram(gStdVertexShader_Textured, gLogoFragmentShader_Textured);

		if (!mcvr->logoProgram) {
			LOGE("Could not create program.");
			IDENTITY("-", "VR", 0, "ret(0x%X)", -2);
			return -2;
		}

		mcvr->logou_mvp_matrix = glGetUniformLocation(mcvr->logoProgram, "u_mvp_matrix");
		checkGlError("glGetUniformLocation u_mvp_matrix");
		mcvr->logou_tex_matrix = glGetUniformLocation(mcvr->logoProgram, "u_tex_matrix");
		checkGlError("glGetUniformLocation u_tex_matrix");
		mcvr->logou_textureSampler = glGetUniformLocation(mcvr->logoProgram, "u_textureSampler");
		checkGlError("glGetUniformLocation u_textureSampler");
		mcvr->logou_alpha = glGetUniformLocation(mcvr->logoProgram, "u_alpha");
		checkGlError("glGetUniformLocation u_alpha");

		mcvr->logoa_position = glGetAttribLocation(mcvr->logoProgram, "a_position");
		checkGlError("glGetAttribLocation a_position");
		mcvr->logoa_texCoord = glGetAttribLocation(mcvr->logoProgram, "a_texCoord");
		checkGlError("glGetAttribLocation a_texCoord");

		glGenTextures(1, &mcvr->logoTexName);

		glBindTexture( GL_TEXTURE_2D, mcvr->logoTexName );
		checkGlError("glBindTexture");

		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		checkGlError("glTexParameteri");
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		checkGlError("glTexParameteri");

		if(mcvr->logoTexName)
		{
			mcvr->eglcontext = eglGetCurrentContext();
			queueTextureForDeletion(mcvr->logoTexName, mcvr->eglcontext);
		}

#endif
		int ret = 0;

		if (NULL == mcvr->surfaceTexture)
		{
			glGenTextures(1, &mcvr->texName);
			mcvr->eglcontext = eglGetCurrentContext();
			ret = NexSurfaceTexture_using_jni::createSurfaceTexture(mcvr->texName, mcvr->surfaceTexture);
			if (0 != ret)
			{
				MC_ERR("Exception occurred while creating surface texture (%d)", ret);
				glDeleteTextures(1, &mcvr->texName);
				mcvr->texName = 0;
#if defined(_SHOW_LOGO_)
				glDeleteTextures(1, &mcvr->logoTexName);
				mcvr->logoTexName = 0;
#endif
				IDENTITY("-", "VR", 0, "ret(0x%X)", -3);
				return -3;
			}
			STATUS(Log::EXTERNAL_CALL, Log::VERBOSE, "->detachFromGLContext");
			ret = mcvr->surfaceTexture->detachFromGLContext();
			STATUS(Log::EXTERNAL_CALL, Log::VERBOSE, "detachFromGLContext->");
			glDeleteTextures(1, &mcvr->texName);
			mcvr->texName = 0;
			if (0 != ret)
			{
				MC_ERR("Exception occurred while detaching from gl context (%d)", ret);
#if defined(_SHOW_LOGO_)
				glDeleteTextures(1, &mcvr->logoTexName);
				mcvr->logoTexName = 0;
#endif
				IDENTITY("-", "VR", 0, "ret(0x%X)", -4);
				return -4;
			}
			mcvr->surfaceTextureDetached = true;
		}
	}
	else if (0 > width || 0 > height)
	{
		int ret = 0;

		if (!mcvr->surfaceTextureDetached && NULL != mcvr->surfaceTexture)
		{
			STATUS(Log::EXTERNAL_CALL, Log::VERBOSE, "->detachFromGLContext");
			ret = mcvr->surfaceTexture->detachFromGLContext();
			STATUS(Log::EXTERNAL_CALL, Log::VERBOSE, "detachFromGLContext->");
			glDeleteTextures(1, &mcvr->texName);
			mcvr->texName = 0;
			if (0 != ret)
			{
				MC_ERR("Exception occurred while detaching from gl context (%d)", ret);
				IDENTITY("-", "VR", 0, "ret(0x%X)", -5);
				return -5;
			}
			mcvr->surfaceTextureDetached = true;
		}

		if(mcvr->program)
		{
			glDeleteProgram(mcvr->program);
			mcvr->program = 0;
		}
#if defined(_SHOW_LOGO_)
		if(mcvr->logoProgram)
		{
			glDeleteProgram(mcvr->logoProgram);
			mcvr->logoProgram = 0;
		}
#endif
	}

	IDENTITY("-", "VR", 0, "ret(0x%X)", retValue);
	return retValue;
}

unsigned int GLDraw(int nMode, void* uUserData)
{
	IDENTITY("+", "VR", uUserData, "mode(%d)", nMode);

	if (NULL == (void *)uUserData)
	{
		MC_ERR("invalid userdata received");
		IDENTITY("-", "VR", uUserData, "ret(0x%X)", -1);
		return -1;
	}

	NexRAL_mcvr *mcvr = (NexRAL_mcvr *)uUserData;

#if defined(_SHOW_LOGO_)
	if (mcvr->drawingLogo || mcvr->drawingLogo2 || mcvr->drawingLogo3)
	{
		mcvr->drawingLogo3 = mcvr->drawingLogo2;
		mcvr->drawingLogo2 = mcvr->drawingLogo;

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		checkGlError("glClearColor");

		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		checkGlError("glClear");

		glUseProgram(mcvr->logoProgram);
		checkGlError("glUseProgram");

		// show logo:
		float top, left, right, bottom;

		int texW = 1;
		int texH = 1;
		while (texW < NEX_LOGO_WIDTH)
			texW = texW << 1;
		while (texH < NEX_LOGO_HEIGHT)
			texH = texH << 1;

		// center it! 50% of screen (depending on aspect ratio of screen)
		float logo_ratio = texW / (float)texH;
		float screen_ratio = 0;
		if (0 != mcvr->surfaceHeight)
		{
			screen_ratio = mcvr->surfaceWidth / (float)mcvr->surfaceHeight;
		}
		else
		{
			screen_ratio = 16 / 9.0f;
		}

		if (screen_ratio < logo_ratio) // 50% by width
		{
			top = -0.5 * screen_ratio / logo_ratio;
			bottom = 0.5 * screen_ratio / logo_ratio;
			left = -0.5;
			right = 0.5;
		}
		else // 50% by height
		{
			top = -0.5;
			bottom = 0.5;
			left = -0.5 * logo_ratio / screen_ratio;
			right = 0.5 * logo_ratio / screen_ratio;
		}

		STATUS(Log::OUTPUT, Log::DEBUG, "T : %3.1f B : %3.1f R : %3.1f : %3.1f %d %d ", top, bottom, right, left, NEX_LOGO_WIDTH, NEX_LOGO_HEIGHT);

		GLfloat spriteVertices[] =
		{
			left, top,
			right, top,
			left, bottom,
			right, bottom
		};

		GLfloat textureCoords[] =
		{
			0.0f, 1.0f,
			1.0f, 1.0f,
			0.0f, 0.0f,
			1.0f, 0.0f
		};

		glActiveTexture(GL_TEXTURE0);
		checkGlError("glActiveTexture");
		glBindTexture(GL_TEXTURE_2D, mcvr->logoTexName);
		checkGlError("glBindTexture");
//		if( nex_logo_data )
		{
			GLubyte *logo = (GLubyte *)malloc(texW*texH*4);
			if (NULL == logo)
			{
				MC_ERR("malloc failed!");
				IDENTITY("-", "VR", uUserData, "ret(0x%X)", -1);
				return -1;
			}
			STATUS(Log::FLOW, Log::INFO, "%p = malloc(%d)", logo, texW*texH*4);
			//memset(logo, 0x00, texW*texH*4);
			unsigned char tmpRGBA[4] = {0x00, 0x00, 0x00, 0xFF};
			for(int i = 0 ; i < texW*texH ; i++)
			{
				memcpy(logo + (i*4), tmpRGBA, 4);
			}

			int logo_offset = ((texH - NEX_LOGO_HEIGHT) / 2) * texW + ((texW - NEX_LOGO_WIDTH) / 2);

			for (int i = 0; i < NEX_LOGO_HEIGHT; ++i)
			{
				memcpy(logo + (logo_offset + i*texW)*4, nex_logo_data + i*NEX_LOGO_WIDTH*4, NEX_LOGO_WIDTH*4);
			}

			//memset(logo, 0xFF, NEX_LOGO_SIZE);
			glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, (const GLvoid*)logo );
			checkGlError("glTexture2D");

			free(logo);
		}

		glUniform1i(mcvr->logou_textureSampler, 0);
		checkGlError("glUniform1i mcvr->logou_textureSampler");

		glEnableVertexAttribArray(mcvr->logoa_position);
		checkGlError("glEnableVertexAttribArray mcvr->logoa_position");

		glEnableVertexAttribArray(mcvr->logoa_texCoord);
		checkGlError("glEnableVertexAttribArray mcvr->logoa_texCoord");

		glUniformMatrix4fv(mcvr->logou_mvp_matrix, 1, GL_FALSE, IDENTITY_MATRIX);
		checkGlError("glUniformMatrix4fv mcvr->logou_mvp_matrix");

        glUniformMatrix4fv(mcvr->logou_tex_matrix, 1, GL_FALSE, IDENTITY_MATRIX);
		checkGlError("glUniformMatrix4fv mcvr->logou_tex_matrix");

		glUniform1f(mcvr->logou_alpha, 1.0f);
		checkGlError("glUniform1f mcvr->logou_alpha");

		glVertexAttribPointer(mcvr->logoa_position, 2, GL_FLOAT, GL_FALSE, 0, spriteVertices);
		checkGlError("glVertexAttribPointer mcvr->logoa_position");

		glVertexAttribPointer(mcvr->logoa_texCoord, 2, GL_FLOAT, GL_FALSE, 0, textureCoords);
		checkGlError("glVertexAttribPointer mcvr->logoa_texCoord");

		glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
		checkGlError("glDrawArray");

		mcvr->drawingLogo = false;

		if (mcvr->drawingLogo2 || mcvr->drawingLogo3) // triple buffering....
		{
			if(mcvr->ralCallbackFunction)
			{
				mcvr->ralCallbackFunction(0x70003, 0, 0, 0, NULL, mcvr->ralCallbackUserData);
			}
		}
	}
	else
#endif	// _SHOW_LOGO_
	if (0 != nMode)
	{ // clear screen
		glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
		checkGlError("glClearColor");

		glClear(GL_COLOR_BUFFER_BIT);
		checkGlError("glClear");
	}
	else if (mcvr->drawingBlank || mcvr->drawingBlank2 || mcvr->drawingBlank3)
	{ // draw blank
		mcvr->drawingBlank3 = mcvr->drawingBlank2;
		mcvr->drawingBlank2 = mcvr->drawingBlank;

		glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
		checkGlError("glClearColor");

		glClear(GL_COLOR_BUFFER_BIT);
		checkGlError("glClear");

		mcvr->drawingBlank = false;

		if (mcvr->drawingBlank2 || mcvr->drawingBlank3) // triple buffering....
		{
			if(mcvr->ralCallbackFunction)
			{
				mcvr->ralCallbackFunction(0x70003, 0, 0, 0, NULL, mcvr->ralCallbackUserData);
			}
		}
	}
	else
	{ // render frames
		GLfloat tex_matrix[16] = {0};
		int ret = 0;

		if (!mcvr->hasNoFrame)
		{
			STATUS(Log::OUTPUT, Log::VERBOSE, "rendering mc/mcvr(%p/%p)", mcvr->mc, mcvr);

			if (mcvr->surfaceTextureDetached)
			{
				glGenTextures(1, &mcvr->texName);
				mcvr->eglcontext = eglGetCurrentContext();

				STATUS(Log::EXTERNAL_CALL, Log::VERBOSE, "->attachToGLContext");
				ret = mcvr->surfaceTexture->attachToGLContext(mcvr->texName);
				STATUS(Log::EXTERNAL_CALL, Log::VERBOSE, "attachToGLContext->");
				if (0 != ret)
				{
					MC_ERR("Exception occurred while attaching to gl context (%d)", ret);
					pthread_mutex_lock(&mcvr->glDrawStatusLock);
					mcvr->glDrawWaiting = true;
					pthread_cond_broadcast(&mcvr->glDrawStatusCondition);
					pthread_mutex_unlock(&mcvr->glDrawStatusLock);

					glDeleteTextures(1, &mcvr->texName);
					mcvr->texName = 0;

					IDENTITY("-", "VR", 0, "ret(0x%X)", -1);
					return -1;
				}
				mcvr->surfaceTextureDetached = false;
			}

			{ // clear canvas
				glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
				checkGlError("glClearColor");

				glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
				checkGlError("glClear");

				glUseProgram(mcvr->program);
				checkGlError("glUseProgram");
			}

			ret = mcvr->surfaceTexture->updateTexImage();
			if (0 != ret)
			{
				MC_ERR("Exception occurred while updating tex image (%d)", ret);
				pthread_mutex_lock(&mcvr->glDrawStatusLock);
				mcvr->glDrawWaiting = true;
				pthread_cond_broadcast(&mcvr->glDrawStatusCondition);
				pthread_mutex_unlock(&mcvr->glDrawStatusLock);
				IDENTITY("-", "VR", 0, "ret(0x%X)", -2);
				return -2;
			}
			ret = mcvr->surfaceTexture->getTransformMatrix(tex_matrix);
			if (0 != ret)
			{
				MC_ERR("Exception occurred while getting transform matrix (%d)", ret);
				pthread_mutex_lock(&mcvr->glDrawStatusLock);
				mcvr->glDrawWaiting = true;
				pthread_cond_broadcast(&mcvr->glDrawStatusCondition);
				pthread_mutex_unlock(&mcvr->glDrawStatusLock);
				IDENTITY("-", "VR", 0, "ret(0x%X)", -3);
				return -3;
			}

			glActiveTexture(GL_TEXTURE0);
			checkGlError("glBindTexture");

			glBindTexture(GL_TEXTURE_EXTERNAL_OES, mcvr->texName);
			checkGlError("glBindTexture");

			glUniform1i(mcvr->u_textureSampler, 0);
			checkGlError("glUniform1i u_textureSampler");

			glEnableVertexAttribArray(mcvr->a_position);
			checkGlError("glEnableVertexAttribArray a_position");

			glEnableVertexAttribArray(mcvr->a_texCoord);
			checkGlError("glEnableVertexAttribArray a_texCoord");

			glUniformMatrix4fv(mcvr->u_mvp_matrix, 1, GL_FALSE, mcvr->mvp);
			checkGlError("glUniformMatrix4fv u_mvp_matrix");

	        glUniformMatrix4fv(mcvr->u_tex_matrix, 1, GL_FALSE, tex_matrix);
			checkGlError("glUniformMatrix4fv u_tex_matrix");

			glUniform1f(mcvr->u_alpha, 1.0f);
			checkGlError("glUniform1f u_alpha");

			glVertexAttribPointer(mcvr->a_position, 2, GL_FLOAT, GL_FALSE, 0, FULL_SCREEN_SURFACE_VERTICES);
			checkGlError("glVertexAttribPointer a_position");

			glVertexAttribPointer(mcvr->a_texCoord, 2, GL_FLOAT, GL_FALSE, 0, FULL_SURFACE_TEXTURE_VERTICES);
			checkGlError("glVertexAttribPointer a_texCoord");

			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			checkGlError("glDrawArrays");
		}
	}

	checkGlError("GLDRAW");

	pthread_mutex_lock(&mcvr->glDrawStatusLock);
	mcvr->glDrawWaiting = true;
	pthread_cond_broadcast(&mcvr->glDrawStatusCondition);
	pthread_mutex_unlock(&mcvr->glDrawStatusLock);

	IDENTITY("-", "VR", 0, "ret(0x%X)", 0);
	return 0;
}

int DisplayBlank(void *pUserData)
{
	IDENTITY("+", "VR", pUserData, "");

	if (NULL == pUserData)
	{
		MC_ERR("invalid userdata received");
		IDENTITY("-", "VR", pUserData, "ret(0x%X)", -1);
		return -1;
	}

	NexRAL_mcvr *mcvr = (NexRAL_mcvr *)pUserData;

	mcvr->drawingBlank = true;

	if(mcvr->ralCallbackFunction)
	{
		mcvr->ralCallbackFunction(0x70003, 0, 0, 0, NULL, mcvr->ralCallbackUserData);
	}

	IDENTITY("-", "VR", 0, "ret(0x%X)", 0);
	return 0;
}

int DisplayLogo(void *pUserData)
{
	IDENTITY("+", "VR", pUserData, "");

	if (NULL == pUserData)
	{
		MC_ERR("invalid userdata received");
		IDENTITY("-", "VR", pUserData, "ret(0x%X)", -1);
		return -1;
	}

	NexRAL_mcvr *mcvr = (NexRAL_mcvr *)pUserData;

#if defined(_SHOW_LOGO_)
	mcvr->drawingLogo = true;

	if(mcvr->ralCallbackFunction)
	{
		mcvr->ralCallbackFunction(0x70003, 0, 0, 0, NULL, mcvr->ralCallbackUserData);
	}
#endif

	IDENTITY("-", "VR", 0, "ret(0x%X)", 0);
	return 0;
}


int surfaceDisplayBlank(void *pUserData)
{
	IDENTITY("+", "VR", pUserData, "");

	if (NULL == pUserData)
	{
		MC_ERR("invalid userdata received");
		IDENTITY("-", "VR", pUserData, "ret(0x%X)", -1);
		return -1;
	}

	NexRAL_mcvr *mcvr = (NexRAL_mcvr *)pUserData;

	if (NULL == mcvr->surfaceTexture)
	{
		MC_ERR("surfaceTexture not properly initialized");
		IDENTITY("-", "VR", pUserData, "ret(0x%X)", -2);
		return -2;
	}

	ANativeWindow *anw = NULL;
	if (0 != mcvr->surfaceTexture->getANativeWindowFromSurfaceObject(anw))
	{
		MC_ERR("error occurred while retrieving anw from surface");
		IDENTITY("-", "VR", pUserData, "ret(0x%X)", -3);
		return -3;
	}

	ANativeWindow_setBuffersGeometry(anw, 128, 128, WINDOW_FORMAT_RGBX_8888);

	int minUndequeuedBufs = 0;
	if (0 > anw->query(anw, 3 /*NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS*/, &minUndequeuedBufs))
	{
		minUndequeuedBufs = 3;
	}

	for (int i = 0; minUndequeuedBufs + 2 > i; ++i)
	{
		ARect inoutbounds;

		inoutbounds.left = 0;
		inoutbounds.right = 128;
		inoutbounds.top = 0;
		inoutbounds.bottom = 128;

		ANativeWindow_Buffer anwBuffer;
		if (0 > ANativeWindow_lock(anw, &anwBuffer, &inoutbounds))
		{
			MC_ERR("error occurred while locking anw");
			IDENTITY("-", "VR", pUserData, "ret(0x%X)", -4);
			return -4;
		}

		// MC_ERR("inoutbounds(blank) post: (%d, %d, %d, %d)", inoutbounds.left, inoutbounds.right, inoutbounds.top, inoutbounds.bottom);

		char *bits = (char *)anwBuffer.bits;
		int pitch = inoutbounds.right - inoutbounds.left;

		memset(bits, 0x00, pitch * (inoutbounds.bottom - inoutbounds.top) * 4);

/*
		for (int y = inoutbounds.top; y < inoutbounds.bottom; ++y)
		{
			for (int x = inoutbounds.left; x < inoutbounds.right; ++x)
			{
				bits[(y*pitch+x)*4] = 0;
				bits[(y*pitch+x)*4 + 1] = 0;
				bits[(y*pitch+x)*4 + 2] = 0;
				bits[(y*pitch+x)*4 + 3] = 255;
			}
		}
*/

		if (0 > ANativeWindow_unlockAndPost(anw))
		{
			MC_ERR("error occurred while unlocking anw");
			IDENTITY("-", "VR", pUserData, "ret(0x%X)", -5);
			return -5;
		}
	}

	if (0 > anw->perform(anw, 14 /*NATIVE_WINDOW_API_DISCONNECT*/, 2 /*NATIVE_WINDOW_API_CPU*/))
	{
		MC_ERR("couldn't disconnect api... may have problems using codec");
	}

	mcvr->surfaceTexture->releaseAcquiredANativeWindow();

	if(mcvr->ralCallbackFunction)
	{
		mcvr->ralCallbackFunction(0x70003, 0, 0, 0, NULL, mcvr->ralCallbackUserData);
	}
	
	IDENTITY("-", "VR", 0, "ret(0x%X)", 0);
	return 0;
}


int surfaceDisplayLogo(void *pUserData)
{
	IDENTITY("+", "VR", pUserData, "");

	if (NULL == pUserData)
	{
		MC_ERR("invalid userdata received");
		IDENTITY("-", "VR", pUserData, "ret(0x%X)", -1);
		return -1;
	}

	NexRAL_mcvr *mcvr = (NexRAL_mcvr *)pUserData;

	if (NULL == mcvr->surfaceTexture)
	{
		MC_ERR("surfaceTexture not properly initialized");
		IDENTITY("-", "VR", pUserData, "ret(0x%X)", -2);
		return -2;
	}

	ANativeWindow *anw = NULL;
	if (0 != mcvr->surfaceTexture->getANativeWindowFromSurfaceObject(anw))
	{
		MC_ERR("error occurred while retrieving anw from surface");
		IDENTITY("-", "VR", pUserData, "ret(0x%X)", -3);
		return -3;
	}

	if (0 > anw->perform(anw, 10 /*NATIVE_WINDOW_SET_SCALING_MODE*/, 1 /*NATIVE_WINDOW_SCALING_MODE_SCALE_TO_WINDOW*/))
	{
		MC_ERR("setting scaling mode failed. logo may look odd...");
	}

	ANativeWindow_setBuffersGeometry(anw, 1280, 720, WINDOW_FORMAT_RGBX_8888);

	int minUndequeuedBufs = 0;
	if (0 > anw->query(anw, 3 /*NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS*/, &minUndequeuedBufs))
	{
		minUndequeuedBufs = 3;
	}

	for (int i = 0; minUndequeuedBufs + 2 > i; ++i)
	{
		ARect inoutbounds;

		//inoutbounds.left = 420;
		//inoutbounds.right = 420+440;
		//inoutbounds.top = 300;
		//inoutbounds.bottom = 300+117;

		inoutbounds.left = 0;
		inoutbounds.right = 1280;
		inoutbounds.top = 0;
		inoutbounds.bottom = 720;

		ANativeWindow_Buffer anwBuffer;
		if (0 > ANativeWindow_lock(anw, &anwBuffer, &inoutbounds))
		{
			MC_ERR("error occurred while locking anw");
			IDENTITY("-", "VR", pUserData, "ret(0x%X)", -4);
			return -4;
		}

		// MC_ERR("inoutbounds(logo) post: (%d, %d, %d, %d)", inoutbounds.left, inoutbounds.right, inoutbounds.top, inoutbounds.bottom);

		char *bits = (char *)anwBuffer.bits;
		int pitch = inoutbounds.right - inoutbounds.left;

		memset(bits, 0x00, pitch * (inoutbounds.bottom - inoutbounds.top) * 4);

/*
		for (int y = inoutbounds.top; y < inoutbounds.bottom; ++y)
		{
			for (int x = inoutbounds.left; x < inoutbounds.right; ++x)
			{
				bits[(y*pitch+x)*4] = 0;
				bits[(y*pitch+x)*4 + 1] = 0;
				bits[(y*pitch+x)*4 + 2] = 0;
				bits[(y*pitch+x)*4 + 3] = 255;
			}
		}
*/

		bits = &bits[(300*pitch + 420)*4];

		// 117(NEX_LOGO_HEIGHT) = (NEX_LOGO_SIZE/NEX_LOGO_WIDTH/4)
		for (int y = 0; y < 117; ++y)
		{
			memcpy(bits, nex_logo_data+y*4*440, 440*4);
			bits += pitch*4;
		}

		if (0 > ANativeWindow_unlockAndPost(anw))
		{
			MC_ERR("error occurred while unlocking anw");
			IDENTITY("-", "VR", pUserData, "ret(0x%X)", -5);
			return -5;
		}
	}

	if (0 > anw->perform(anw, 14 /*NATIVE_WINDOW_API_DISCONNECT*/, 2 /*NATIVE_WINDOW_API_CPU*/))
	{
		MC_ERR("couldn't disconnect api... may have problems using codec");
	}

	mcvr->surfaceTexture->releaseAcquiredANativeWindow();

	if(mcvr->ralCallbackFunction)
	{
		mcvr->ralCallbackFunction(0x70003, 0, 0, 0, NULL, mcvr->ralCallbackUserData);
	}
	IDENTITY("-", "VR", 0, "ret(0x%X)", 0);
	return 0;
}

}; // namespace Renderer
}; // namespace Video
}; // namespace Nex_MC

namespace { // (anon5)
	VIDEO_RALBODY_FUNCTION_ST g_hRAL_mc =
		{ 0		//	unsigned int	uiCPUInfo;
		, 0		// 	unsigned int	uiPlatformInfo;
		, 0x20	// 	unsigned int	uiRenderInfo;		//NEX_USE_RENDER_OPENGL in NexPlayerSDK_for_Download\NexPlayer_Porting\Porting_Android\deviceInfo.h
		, 0		// 	unsigned int	uiDevColorFormat;
		, 0		// 	unsigned int	uiIOMXColorFormat;

		, Nex_MC::Video::Renderer::GetProperty		//	NEXRALBody_Video_getProperty 			fnNexRALBody_Video_getProperty;
		, Nex_MC::Video::Renderer::SetProperty		//	NEXRALBody_Video_setProperty			fnNexRALBody_Video_setProperty;
		, Nex_MC::Video::Renderer::Init				//	NEXRALBody_Video_init					fnNexRALBody_Video_init;
		, Nex_MC::Video::Renderer::Deinit			//	NEXRALBody_Video_deinit					fnNexRALBody_Video_deinit;
		, Nex_MC::Video::Renderer::Pause			//	NEXRALBody_Video_pause					fnNexRALBody_Video_pause;
		, Nex_MC::Video::Renderer::Resume			//	NEXRALBody_Video_resume					fnNexRALBody_Video_resume;
		, Nex_MC::Video::Renderer::Flush			//	NEXRALBody_Video_flush					fnNexRALBody_Video_flush;
		, Nex_MC::Video::Renderer::Display			//	NEXRALBody_Video_display				fnNexRALBody_Video_display;
		, Nex_MC::Video::Renderer::Create			//	NEXRALBody_Video_create					fnNexRALBody_Video_create;
		, Nex_MC::Video::Renderer::Delete			//	NEXRALBody_Video_delete					fnNexRALBody_Video_delete;
		, NULL										//	NEXRALBody_Video_prepareSurface			fnNexRALBody_Video_prepareSurface;
		, NULL										//	NEXRALBody_Video_setRenderOption		fnNexRALBody_Video_setRenderOption;
		, Nex_MC::Video::Renderer::SetOutputPos		//	NEXRALBody_Video_setOutputPos			fnNexRALBody_Video_setOutputPos;
		, Nex_MC::Video::Renderer::OnOff			//	NEXRALBody_Video_OnOff					fnNexRALBody_Video_OnOff;
		, NULL										//	NEXRALBody_Video_Capture				fnNexRALBody_Video_Capture;
		, NULL										//	NEXRALBody_Video_SetBitmap				fnNexRALBody_Video_SetBitmap;
		, NULL										//	NEXRALBody_Video_FillBitmap				fnNexRALBody_Video_FillBitmap;
		, Nex_MC::Video::Renderer::InitOpenGL		//	NEXRALBody_Video_GLInit					fnNexRALBody_Video_GLInit;
		, Nex_MC::Video::Renderer::GLDraw			//	NEXRALBody_Video_GLDraw					fnNexRALBody_Video_GLDraw;
		, NULL										//	NEXRALBody_Video_SetContrastBrightness	fnNexRALBody_Video_SetContrastBrightness;
		, Nex_MC::Video::Renderer::DisplayBlank		//	NEXRALBody_Video_DisplayBlank			fnNexRALBody_Video_DisplayBlank; //for ics native window.
		, Nex_MC::Video::Renderer::DisplayLogo		//	NEXRALBody_Video_DisplayBlank			fnNexRALBody_Video_DisplayLogo; //for ics native window.
		, NULL
		};

	VIDEO_RALBODY_FUNCTION_ST g_hRAL_surface_mc =
	{ 	0		//	unsigned int	uiCPUInfo;
		, 0		// 	unsigned int	uiPlatformInfo;
		, 0x04	// 	unsigned int	uiRenderInfo;		//NEX_USE_RENDER_OPENGL in NexPlayerSDK_for_Download\NexPlayer_Porting\Porting_Android\deviceInfo.h
		, 0		// 	unsigned int	uiDevColorFormat;
		, 0		// 	unsigned int	uiIOMXColorFormat;

		, Nex_MC::Video::Renderer::GetProperty		//	NEXRALBody_Video_getProperty 			fnNexRALBody_Video_getProperty;
		, Nex_MC::Video::Renderer::SetProperty		//	NEXRALBody_Video_setProperty			fnNexRALBody_Video_setProperty;
		, Nex_MC::Video::Renderer::Init				//	NEXRALBody_Video_init					fnNexRALBody_Video_init;
		, Nex_MC::Video::Renderer::Deinit			//	NEXRALBody_Video_deinit					fnNexRALBody_Video_deinit;
		, Nex_MC::Video::Renderer::Pause			//	NEXRALBody_Video_pause					fnNexRALBody_Video_pause;
		, Nex_MC::Video::Renderer::Resume			//	NEXRALBody_Video_resume					fnNexRALBody_Video_resume;
		, Nex_MC::Video::Renderer::Flush			//	NEXRALBody_Video_flush					fnNexRALBody_Video_flush;
		, Nex_MC::Video::Renderer::surfaceDisplay	//	NEXRALBody_Video_display				fnNexRALBody_Video_display;
		, Nex_MC::Video::Renderer::Create			//	NEXRALBody_Video_create					fnNexRALBody_Video_create;
		, Nex_MC::Video::Renderer::surfaceDelete	//	NEXRALBody_Video_delete					fnNexRALBody_Video_delete;
		, Nex_MC::Video::Renderer::prepareSurface	//	NEXRALBody_Video_prepareSurface			fnNexRALBody_Video_prepareSurface;
		, NULL										//	NEXRALBody_Video_setRenderOption		fnNexRALBody_Video_setRenderOption;
		, NULL										//	NEXRALBody_Video_setOutputPos			fnNexRALBody_Video_setOutputPos;
		, Nex_MC::Video::Renderer::OnOff			//	NEXRALBody_Video_OnOff					fnNexRALBody_Video_OnOff;
		, NULL										//	NEXRALBody_Video_Capture				fnNexRALBody_Video_Capture;
		, NULL										//	NEXRALBody_Video_SetBitmap				fnNexRALBody_Video_SetBitmap;
		, NULL										//	NEXRALBody_Video_FillBitmap				fnNexRALBody_Video_FillBitmap;
		, NULL										//	NEXRALBody_Video_GLInit					fnNexRALBody_Video_GLInit;
		, NULL										//	NEXRALBody_Video_GLDraw					fnNexRALBody_Video_GLDraw;
		, NULL										//	NEXRALBody_Video_SetContrastBrightness	fnNexRALBody_Video_SetContrastBrightness;
		, Nex_MC::Video::Renderer::surfaceDisplayBlank	//	NEXRALBody_Video_DisplayBlank			fnNexRALBody_Video_DisplayBlank; //for ics native window.
		, Nex_MC::Video::Renderer::surfaceDisplayLogo	//	NEXRALBody_Video_DisplayBlank			fnNexRALBody_Video_DisplayLogo; //for ics native window.
		, NULL
	};
}; // namespace (anon5)


//##############################################################################
using namespace Nex_MC::Log;

#ifndef NEXCAL_PROPERTY_OUTPUTBUFFER_TYPE_NOT_MEMORY
#define NEXCAL_PROPERTY_OUTPUTBUFFER_TYPE_NOT_MEMORY 1
#endif

EXPORTED
unsigned int getNexRAL_MC_Version()
{
	const char *build_number = xstr(NEXCRALBODY_MC_VERSION_BUILD);
	return (((NEXCRALBODY_MC_VERSION_MAJOR)*100 + NEXCRALBODY_MC_VERSION_MINOR)*100 + NEXCRALBODY_MC_VERSION_PATCH)*100 + (unsigned int)(build_number[0]);
}

EXPORTED
void SetJavaVMforRender(JavaVM* javaVM)
{
	Nex_MC::Utils::JNI::setJavaVMfromExternal(javaVM);
}

EXPORTED
VIDEO_RALBODY_FUNCTION_ST *getNexRAL_MC_VideoRenderer(
	  unsigned int uBufferFormat
	, unsigned int API_Version
	, unsigned int nexRAL_API_MAJOR
	, unsigned int nexRAL_API_MINOR
	, unsigned int nexRAL_API_PATCH_NUM
	)
{
	Nex_MC::Utils::initializeAll();

	LOGW("========================================================\n");
	LOGW("NexRALBody_MC Version : %d.%d.%d (%s)\n",	NEXCRALBODY_MC_VERSION_MAJOR,
													NEXCRALBODY_MC_VERSION_MINOR,
													NEXCRALBODY_MC_VERSION_PATCH,
													xstr(NEXCRALBODY_MC_VERSION_MAJOR)
													xstr(NEXCRALBODY_MC_VERSION_MINOR)
													xstr(NEXCRALBODY_MC_VERSION_PATCH)
													xstr(NEXCRALBODY_MC_VERSION_BUILD));
	LOGW("NexRALBody_MC Information : %s\n", NEXCRALBODY_MC_INFORMATION);
	LOGW("========================================================\n");

	// initialize time:
	Nex_MC::Utils::Time::GetTickCount();

	STATUS(FLOW, INFO, "+ bufferFormat(0x%X)", uBufferFormat);

	STATUS(INPUT, INFO
		, "incoming info API Version:%d, nexRAL_MAJOR:%d, nexRAL_MINOR:%d, nexRAL_PATCH_NUM:%d"
		, API_Version, nexRAL_API_MAJOR, nexRAL_API_MINOR, nexRAL_API_PATCH_NUM);

	if (NEXRAL_MC_API_VERSION != API_Version)
	{
		STATUS(INPUT, WARNING
			, "WARNING: API Version does not match! (mine:%u yours:%u)", NEXRAL_MC_API_VERSION, API_Version);
	}

	if (   NEXRAL_VERSION_MAJOR != nexRAL_API_MAJOR
		|| NEXRAL_VERSION_MINOR != nexRAL_API_MINOR
		|| NEXRAL_VERSION_PATCH != nexRAL_API_PATCH_NUM )
	{
		STATUS(INPUT, WARNING
			, "WARNING: RAL API Version does not match! (mine:%x.%x.%x yours:%x.%x.%x)"
			, NEXRAL_VERSION_MAJOR, NEXRAL_VERSION_MINOR, NEXRAL_VERSION_PATCH
			, nexRAL_API_MAJOR, nexRAL_API_MINOR, nexRAL_API_PATCH_NUM);
	}

	if (NEXCAL_PROPERTY_OUTPUTBUFFER_TYPE_NOT_MEMORY != uBufferFormat)
	{
		MC_ERR("this RALBody does not support uBufferFormat(0x%X)", uBufferFormat);
		return NULL;
	}

	STATUS(FLOW, DEBUG, "-");
	return &g_hRAL_mc;
}

EXPORTED
VIDEO_RALBODY_FUNCTION_ST *getNexRAL_MC_SurfaceVideoRenderer(
	  unsigned int uBufferFormat
	, unsigned int API_Version
	, unsigned int nexRAL_API_MAJOR
	, unsigned int nexRAL_API_MINOR
	, unsigned int nexRAL_API_PATCH_NUM
	)
{
	Nex_MC::Utils::initializeAll();

	LOGW("========================================================\n");
	LOGW("NexRALBody_MC Version : %d.%d.%d (%s)\n",	NEXCRALBODY_MC_VERSION_MAJOR,
													NEXCRALBODY_MC_VERSION_MINOR,
													NEXCRALBODY_MC_VERSION_PATCH,
													xstr(NEXCRALBODY_MC_VERSION_MAJOR)
													xstr(NEXCRALBODY_MC_VERSION_MINOR)
													xstr(NEXCRALBODY_MC_VERSION_PATCH)
													xstr(NEXCRALBODY_MC_VERSION_BUILD));
	LOGW("NexRALBody_MC Information : %s\n", NEXCRALBODY_MC_INFORMATION);
	LOGW("========================================================\n");

	// initialize time:
	Nex_MC::Utils::Time::GetTickCount();

	STATUS(FLOW, INFO, "+ bufferFormat(0x%X)", uBufferFormat);

	STATUS(INPUT, INFO
		, "incoming info API Version:%d, nexRAL_MAJOR:%d, nexRAL_MINOR:%d, nexRAL_PATCH_NUM:%d"
		, API_Version, nexRAL_API_MAJOR, nexRAL_API_MINOR, nexRAL_API_PATCH_NUM);

	if (NEXRAL_MC_API_VERSION != API_Version)
	{
		STATUS(INPUT, WARNING
			, "WARNING: API Version does not match! (mine:%u yours:%u)", NEXRAL_MC_API_VERSION, API_Version);
	}

	if (   NEXRAL_VERSION_MAJOR != nexRAL_API_MAJOR
		|| NEXRAL_VERSION_MINOR != nexRAL_API_MINOR
		|| NEXRAL_VERSION_PATCH != nexRAL_API_PATCH_NUM )
	{
		STATUS(INPUT, WARNING
			, "WARNING: RAL API Version does not match! (mine:%x.%x.%x yours:%x.%x.%x)"
			, NEXRAL_VERSION_MAJOR, NEXRAL_VERSION_MINOR, NEXRAL_VERSION_PATCH
			, nexRAL_API_MAJOR, nexRAL_API_MINOR, nexRAL_API_PATCH_NUM);
	}

	if (NEXCAL_PROPERTY_OUTPUTBUFFER_TYPE_NOT_MEMORY != uBufferFormat)
	{
		MC_ERR("this RALBody does not support uBufferFormat(0x%X)", uBufferFormat);
		return NULL;
	}

	STATUS(FLOW, DEBUG, "-");
	return &g_hRAL_surface_mc;
}

#if defined(NEX_ADD_NW_DEFS)
extern "C" {
EXPORTED
VIDEO_RALBODY_FUNCTION_ST *getNexRAL_NWGL_VideoRenderer(
	  unsigned int uBufferFormat
	, unsigned int API_Version
	, unsigned int nexRAL_API_MAJOR
	, unsigned int nexRAL_API_MINOR
	, unsigned int nexRAL_API_PATCH_NUM
	)
{
	return getNexRAL_MC_VideoRenderer(uBufferFormat, API_Version, nexRAL_API_MAJOR, nexRAL_API_MINOR, nexRAL_API_PATCH_NUM);
}

EXPORTED
VIDEO_RALBODY_FUNCTION_ST *getNexRAL_NW_VideoRenderer(
	  unsigned int uBufferFormat
	, unsigned int API_Version
	, unsigned int nexRAL_API_MAJOR
	, unsigned int nexRAL_API_MINOR
	, unsigned int nexRAL_API_PATCH_NUM
	)
{
	return getNexRAL_MC_VideoRenderer(uBufferFormat, API_Version, nexRAL_API_MAJOR, nexRAL_API_MINOR, nexRAL_API_PATCH_NUM);
}
};
#endif

