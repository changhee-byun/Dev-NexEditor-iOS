#include <jni.h>
#include <stdio.h>
#include <android/log.h>

#define  LOG_TAG    "NexLayer"

#define  LOGI(...)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

#include <math.h>
#include <string>
#include <vector>
#include <map>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

using namespace std;

#include "NEXVIDEOEDITOR_Interface.h"

#include "common_def.hpp"
#include "chromakey_info.hpp"
#include "matrix.hpp"
#include "render_interface.hpp"
#include "render_resource.hpp"
#include "shader_decorator.hpp"
#include "base_shader.hpp"
#include "colorconv_shader.hpp"
#include "useralpha_shader.hpp"
#include "chromakey_shader.hpp"
#include "mask_shader.hpp"
#include "colorlut_shader.hpp"
#include "blur_shader.hpp"
#include "mosaic_shader.hpp"
#include "colormatrix.hpp"
#include "statestack_entry.hpp"
#include "simplerenderer.hpp"
#include "renderer.hpp"

#ifdef FOR_PROJECT_Kinemaster
#define NEXLAYERPREFIX(x)	Java_com_nextreaming_nexvideoeditor_NexLayerRenderer_ ## x
#else
#define NEXLAYERPREFIX(x)	Java_com_nexstreaming_kminternal_nexvideoeditor_NexLayerRenderer_ ## x
#endif

//static renderer gLayerRenderer;
static renderer *gLayerRenderer = NULL;

extern "C" {
	JNIEXPORT void JNICALL NEXLAYERPREFIX(createRenderer)(JNIEnv * env, jobject obj);
	JNIEXPORT void JNICALL NEXLAYERPREFIX(removeRenderer)(JNIEnv * env, jobject obj);

	JNIEXPORT jfloatArray JNICALL NEXLAYERPREFIX(getTexMatrix)(JNIEnv * env, jobject obj);
	JNIEXPORT jfloat JNICALL NEXLAYERPREFIX(getOutputWidth)(JNIEnv * env, jobject obj);
	JNIEXPORT jfloat JNICALL NEXLAYERPREFIX(getOutputHeight)(JNIEnv * env, jobject obj);
	JNIEXPORT jint JNICALL NEXLAYERPREFIX(getCurrentTime)(JNIEnv * env, jobject obj);
	JNIEXPORT void JNICALL NEXLAYERPREFIX(setColorMatrix)(JNIEnv * env, jobject obj, jfloatArray color_matrix_array);
	JNIEXPORT jobject  JNICALL NEXLAYERPREFIX(getColorMatrix)(JNIEnv * env, jobject obj);
	JNIEXPORT void JNICALL NEXLAYERPREFIX(save)(JNIEnv * env, jobject obj);
	JNIEXPORT void JNICALL NEXLAYERPREFIX(restore)(JNIEnv * env, jobject obj);
	JNIEXPORT void JNICALL NEXLAYERPREFIX(translate)(JNIEnv * env, jobject obj, jfloat x, jfloat y, jfloat z);
	JNIEXPORT void JNICALL NEXLAYERPREFIX(translateXY)(JNIEnv * env, jobject obj, jfloat x, jfloat y);
	JNIEXPORT void JNICALL NEXLAYERPREFIX(scale)(JNIEnv * env, jobject obj, jfloat x, jfloat y, jfloat cx, jfloat cy);
	JNIEXPORT void JNICALL NEXLAYERPREFIX(scaleXY)(JNIEnv * env, jobject obj, jfloat x, jfloat y);
	JNIEXPORT void JNICALL NEXLAYERPREFIX(scaleXYZ)(JNIEnv * env, jobject obj, jfloat x, jfloat y, jfloat z);
	JNIEXPORT void JNICALL NEXLAYERPREFIX(rotateAroundAxis)(JNIEnv * env, jobject obj, jfloat a, jfloat x, jfloat y, jfloat z);
	JNIEXPORT jfloatArray JNICALL NEXLAYERPREFIX(getMatrix)(JNIEnv * env, jobject obj);
	JNIEXPORT void JNICALL NEXLAYERPREFIX(rotate)(JNIEnv * env, jobject obj, jfloat a, jfloat cx, jfloat cy);
	JNIEXPORT void JNICALL NEXLAYERPREFIX(setAlpha)(JNIEnv * env, jobject obj, jfloat alpha);
	JNIEXPORT void JNICALL NEXLAYERPREFIX(setCurrentTime)(JNIEnv * env, jobject obj, jint currentTime);
	JNIEXPORT void JNICALL NEXLAYERPREFIX(setShaderAndParam)(JNIEnv * env, jobject obj, jboolean is_export);
	JNIEXPORT void JNICALL NEXLAYERPREFIX(setFrameDimensions)(JNIEnv * env, jobject obj, jint width, jint height);
	JNIEXPORT void JNICALL NEXLAYERPREFIX(setScreenDimenisions)(JNIEnv * env, jobject obj, jint width, jint height);
	JNIEXPORT void JNICALL NEXLAYERPREFIX(preRender)(JNIEnv * env, jobject obj);
	JNIEXPORT void JNICALL NEXLAYERPREFIX(postRender)(JNIEnv * env, jobject obj);
	JNIEXPORT void JNICALL NEXLAYERPREFIX(setChromakeyViewMaskEnabled)(JNIEnv * env, jobject obj, jboolean maskOnOff);
	JNIEXPORT jint JNICALL NEXLAYERPREFIX(getChromakeyViewMaskEnabled)(JNIEnv * env, jobject obj);
	JNIEXPORT void JNICALL NEXLAYERPREFIX(setChromakeyEnabled)(JNIEnv * env, jobject obj, jboolean enable);
	JNIEXPORT void JNICALL NEXLAYERPREFIX(setChromakeyColor)(JNIEnv * env, jobject obj, jint color, jfloat clipFg, jfloat clipBg, jfloat bx0, jfloat by0, jfloat bx1, jfloat by1);
	JNIEXPORT void JNICALL NEXLAYERPREFIX(setHBlurEnabled)(JNIEnv * env, jobject obj, jboolean enable);
	JNIEXPORT void JNICALL NEXLAYERPREFIX(setVBlurEnabled)(JNIEnv * env, jobject obj, jboolean enable);
	JNIEXPORT void JNICALL NEXLAYERPREFIX(setMosaicEnabled)(JNIEnv * env, jobject obj, jboolean enable);
	JNIEXPORT void JNICALL NEXLAYERPREFIX(setEffectStrength)(JNIEnv * env, jobject obj, jfloat strength);
	JNIEXPORT void JNICALL NEXLAYERPREFIX(setEffectTextureSize)(JNIEnv * env, jobject obj, jint width, jint height);
	JNIEXPORT jfloat JNICALL NEXLAYERPREFIX(getScreenDimensionWidth)(JNIEnv * env, jobject obj);
	JNIEXPORT jfloat JNICALL NEXLAYERPREFIX(getScreenDimensionHeight)(JNIEnv * env, jobject obj);
	JNIEXPORT void JNICALL NEXLAYERPREFIX(setMaskEnabled)(JNIEnv * env, jobject obj, jboolean enable);
	JNIEXPORT jboolean JNICALL NEXLAYERPREFIX(getMaskEnabled)(JNIEnv * env, jobject obj);
	JNIEXPORT void JNICALL NEXLAYERPREFIX(setMaskTexID)(JNIEnv * env, jobject obj, jint texid);
	JNIEXPORT jfloat JNICALL NEXLAYERPREFIX(getAlpha)(JNIEnv * env, jobject obj);
	JNIEXPORT void JNICALL NEXLAYERPREFIX(setAlphaTestValue)(JNIEnv * env, jobject obj, jfloat alpha);
	JNIEXPORT void JNICALL NEXLAYERPREFIX(resetMatrix)(JNIEnv * env, jobject obj);
	JNIEXPORT void JNICALL NEXLAYERPREFIX(setRenderTarget)(JNIEnv * env, jobject obj, jint rendertarget);
	JNIEXPORT void JNICALL NEXLAYERPREFIX(setZTestMode)(JNIEnv * env, jobject obj);
	JNIEXPORT void JNICALL NEXLAYERPREFIX(setZTest)(JNIEnv * env, jobject obj);
	JNIEXPORT void JNICALL NEXLAYERPREFIX(releaseZTest)(JNIEnv * env, jobject obj);
	JNIEXPORT void JNICALL NEXLAYERPREFIX(releaseZTestMode)(JNIEnv * env, jobject obj);
	JNIEXPORT void JNICALL NEXLAYERPREFIX(clearMask)(JNIEnv * env, jobject obj);
	JNIEXPORT void JNICALL NEXLAYERPREFIX(clearMasktoColor)(JNIEnv * env, jobject obj, jint color);
	JNIEXPORT void JNICALL NEXLAYERPREFIX(setRenderMode)(JNIEnv * env, jobject obj, jboolean is_export);
	JNIEXPORT jint JNICALL NEXLAYERPREFIX(getRenderMode)(JNIEnv * env, jobject obj);
	JNIEXPORT void JNICALL NEXLAYERPREFIX(setLUT)(JNIEnv * env, jobject obj, jint tex_id);
	JNIEXPORT void JNICALL NEXLAYERPREFIX(setLUTEnable)(JNIEnv * env, jobject obj, jboolean enable);
	JNIEXPORT void JNICALL NEXLAYERPREFIX(drawBitmap)(JNIEnv * env, jobject obj, jint img_id, jfloat left, jfloat top, jfloat right, jfloat bottom);
	JNIEXPORT void JNICALL NEXLAYERPREFIX(drawBitmapFlip)(JNIEnv * env, jobject obj, jint img_id, jfloat left, jfloat top, jfloat right, jfloat bottom, jint flipmode);
	JNIEXPORT void JNICALL NEXLAYERPREFIX(drawBitmapRepeat)(JNIEnv * env, jobject obj, jint img_id, jfloat left, jfloat top, jfloat right, jfloat bottom, jfloat repeat_x, jfloat repeat_y);
	JNIEXPORT void JNICALL NEXLAYERPREFIX(drawRenderItem)(JNIEnv * env, jobject obj, jint effect_id, jstring usersettings, jint current_time, jint start_time, jint end_time, jfloat left, jfloat top, jfloat right, jfloat bottom, jfloat alpha, jboolean mask_enabled);
	JNIEXPORT void JNICALL NEXLAYERPREFIX(drawRenderItemFlip)(JNIEnv * env, jobject obj, jint effect_id, jstring usersettings, jint current_time, jint start_time, jint end_time, jfloat left, jfloat top, jfloat right, jfloat bottom, jfloat alpha, jboolean mask_enabled, jint flipmode);
    JNIEXPORT void JNICALL NEXLAYERPREFIX(drawRenderItemBlend)(JNIEnv * env, jobject obj, jint effect_id, jint tex_id_for_second, jint type, jstring usersettings, jint current_time, jint start_time, jint end_time, jfloat x, jfloat y, jfloat w, jfloat h, jfloat alpha, jboolean mask_enabled);
	JNIEXPORT void JNICALL NEXLAYERPREFIX(drawRenderItemBlendOverlay)(JNIEnv * env, jobject obj, jint effect_id_for_blend, jint effect_id, jstring usersettings_for_blend, jstring usersettings, jint current_time, jint start_time, jint end_time, jfloat left, jfloat top, jfloat right, jfloat bottom, jfloat alpha, jboolean mask_enabled);
	JNIEXPORT void JNICALL NEXLAYERPREFIX(drawRenderItemBlendFlip)(JNIEnv * env, jobject obj, jint effect_id, jint tex_id_for_second, jstring usersettings, jint current_time, jint start_time, jint end_time, jfloat x, jfloat y, jfloat w, jfloat h, jfloat alpha, jboolean mask_enabled, jint flipmode);
	JNIEXPORT void JNICALL NEXLAYERPREFIX(drawRenderItemBitmap)(JNIEnv * env, jobject obj, jint tex_id_for_second, jint effect_id, jstring usersettings, jint current_time, jint start_time, jint end_time, jfloat x, jfloat y, jfloat w, jfloat h, jfloat alpha, jboolean mask_enabled);
	JNIEXPORT void JNICALL NEXLAYERPREFIX(drawRenderItemBitmapFlip)(JNIEnv * env, jobject obj, jint tex_id_for_second, jint effect_id, jstring usersettings, jint current_time, jint start_time, jint end_time, jfloat x, jfloat y, jfloat w, jfloat h, jfloat alpha, jboolean mask_enabled, jint flipmode);
	JNIEXPORT void JNICALL NEXLAYERPREFIX(fillRect)(JNIEnv * env, jobject obj, jint fill_rect_tex_id, jint color, jfloat left, jfloat top, jfloat right, jfloat bottom);
	JNIEXPORT void JNICALL NEXLAYERPREFIX(clearMaskRegion)(JNIEnv * env, jobject obj, jint fill_rect_tex_id, jint color, jfloat left, jfloat top, jfloat right, jfloat bottom);
	JNIEXPORT void JNICALL NEXLAYERPREFIX(presetForDrawing)(JNIEnv * env, jobject obj, jfloat left, jfloat top, jfloat right, jfloat bottom, jint type);
	JNIEXPORT jfloat JNICALL NEXLAYERPREFIX(getEffectStrength)(JNIEnv * env, jobject obj);
	JNIEXPORT void JNICALL NEXLAYERPREFIX(drawVideo)(JNIEnv * env, jobject obj, jint clipid, jfloat x, jfloat y, jfloat w, jfloat h);
	JNIEXPORT void JNICALL NEXLAYERPREFIX(drawDirect)(JNIEnv * env, jobject obj, jint texname, jint type, jfloat x, jfloat y, jfloat w, jfloat h, jint flipmode);
	JNIEXPORT void JNICALL NEXLAYERPREFIX(drawDirectFlip)(JNIEnv * env, jobject obj, jint texname, jfloat x, jfloat y, jfloat w, jfloat h, jint flipmode);
	JNIEXPORT void JNICALL NEXLAYERPREFIX(setEffectTextureWidth)(JNIEnv * env, jobject obj, jfloat width);
	JNIEXPORT void JNICALL NEXLAYERPREFIX(setEffectTextureHeight)(JNIEnv * env, jobject obj, jfloat height);
	JNIEXPORT jfloat JNICALL NEXLAYERPREFIX(getEffectTextureWidth)(JNIEnv * env, jobject obj);
	JNIEXPORT jfloat JNICALL NEXLAYERPREFIX(getEffectTextureHeight)(JNIEnv * env, jobject obj);
	JNIEXPORT jint JNICALL NEXLAYERPREFIX(getRenderTarget)(JNIEnv * env, jobject obj);
	JNIEXPORT void JNICALL NEXLAYERPREFIX(setColorLookupTable)(JNIEnv * env, jobject obj, jint img_id, jfloat x, jfloat y);
	JNIEXPORT void JNICALL NEXLAYERPREFIX(setHue)(JNIEnv * env, jobject obj, jfloat hue);
	JNIEXPORT jfloat JNICALL NEXLAYERPREFIX(getHue)(JNIEnv * env, jobject obj);
	JNIEXPORT void JNICALL NEXLAYERPREFIX(drawNexEDL)(JNIEnv * env, jobject obj, jint effect_id, jstring usersettings, jint current_time, jint start_time, jint end_time, jfloat left, jfloat top, jfloat right, jfloat bottom, jfloat alpha, jint mode);

	JNIEXPORT void JNICALL NEXLAYERPREFIX(setBlendFuncNormal)(JNIEnv * env, jobject obj);
	JNIEXPORT void JNICALL NEXLAYERPREFIX(setBlendFuncMultiply)(JNIEnv * env, jobject obj);
};

JNIEXPORT void JNICALL NEXLAYERPREFIX(createRenderer)(JNIEnv * env, jobject obj)
{
	if( gLayerRenderer )
	{
		SAFE_DELETE(gLayerRenderer);
	}

	gLayerRenderer = new renderer();
}

JNIEXPORT void JNICALL NEXLAYERPREFIX(removeRenderer)(JNIEnv * env, jobject obj)
{
	if( gLayerRenderer )
	{
		SAFE_DELETE(gLayerRenderer);
	}
	gLayerRenderer = NULL;
}
JNIEXPORT jfloatArray JNICALL NEXLAYERPREFIX(getTexMatrix)(JNIEnv * env, jobject obj)
{
	jfloatArray outJNIArray;

	outJNIArray = env->NewFloatArray(16);

	env->SetFloatArrayRegion(outJNIArray, 0, 16, gLayerRenderer->getTexMatrix());
	
	return outJNIArray;
}

JNIEXPORT jfloat JNICALL NEXLAYERPREFIX(getOutputWidth)(JNIEnv * env, jobject obj)
{
	return gLayerRenderer->getFrameWidth();
}

JNIEXPORT jfloat JNICALL NEXLAYERPREFIX(getOutputHeight)(JNIEnv * env, jobject obj)
{
	return gLayerRenderer->getFrameHeight();
}

JNIEXPORT void JNICALL NEXLAYERPREFIX(setEffectTextureWidth)(JNIEnv * env, jobject obj, jfloat width)
{
	gLayerRenderer->setEffectTextureSizeW(width);
}

JNIEXPORT void JNICALL NEXLAYERPREFIX(setEffectTextureHeight)(JNIEnv * env, jobject obj, jfloat height)
{
	gLayerRenderer->setEffectTextureSizeH(height);
}

JNIEXPORT jfloat JNICALL NEXLAYERPREFIX(getEffectTextureWidth)(JNIEnv * env, jobject obj)
{
	return gLayerRenderer->getEffectTextureWidth();
}

JNIEXPORT jfloat JNICALL NEXLAYERPREFIX(getEffectTextureHeight)(JNIEnv * env, jobject obj)
{
	return gLayerRenderer->getEffectTextureHeight();
}

JNIEXPORT jint JNICALL NEXLAYERPREFIX(getCurrentTime)(JNIEnv * env, jobject obj)
{
	return gLayerRenderer->getCurrentTime();
}

JNIEXPORT void JNICALL NEXLAYERPREFIX(setColorMatrix)(JNIEnv * env, jobject obj, jfloatArray color_matrix_array)
{
    ColorMatrix matrix;
    env->GetFloatArrayRegion(color_matrix_array, 0, 20, matrix.matrix_);
	gLayerRenderer->setColorMatrix(matrix);
}

JNIEXPORT jobject  JNICALL NEXLAYERPREFIX(getColorMatrix)(JNIEnv * env, jobject obj)
{
	jclass      classColorMatrix;
	jmethodID	classColorMatrix_construct;

	classColorMatrix = env->FindClass("android/graphics/ColorMatrix");
	if(classColorMatrix == NULL ){

		return NULL;
    }

    classColorMatrix_construct = env->GetMethodID(classColorMatrix, "<init>", "([F)V");
	if( classColorMatrix_construct == NULL )
	{
		env->DeleteLocalRef(classColorMatrix);
		return NULL;
	}

    jfloatArray color_matrix_array = env->NewFloatArray(20);
    
    env->SetFloatArrayRegion(color_matrix_array, 0, 20, gLayerRenderer->getColorMatrix().matrix_);

    jobject color_matrix_obj = env->NewObject(classColorMatrix, classColorMatrix_construct, color_matrix_array);

    env->DeleteLocalRef(classColorMatrix);

	return color_matrix_obj;
}

JNIEXPORT void JNICALL NEXLAYERPREFIX(save)(JNIEnv * env, jobject obj)
{
	gLayerRenderer->save();
}

JNIEXPORT void JNICALL NEXLAYERPREFIX(restore)(JNIEnv * env, jobject obj)
{
	gLayerRenderer->restore();
}

JNIEXPORT void JNICALL NEXLAYERPREFIX(translate)(JNIEnv * env, jobject obj, jfloat x, jfloat y, jfloat z)
{
	gLayerRenderer->translate(x, y, z);
}

JNIEXPORT void JNICALL NEXLAYERPREFIX(translateXY)(JNIEnv * env, jobject obj, jfloat x, jfloat y)
{
	gLayerRenderer->translate(x, y);
}

JNIEXPORT void JNICALL NEXLAYERPREFIX(scale)(JNIEnv * env, jobject obj, jfloat x, jfloat y, jfloat cx, jfloat cy)
{
	gLayerRenderer->scale(x, y, cx, cy);
}

JNIEXPORT void JNICALL NEXLAYERPREFIX(scaleXY)(JNIEnv * env, jobject obj, jfloat x, jfloat y)
{
	gLayerRenderer->scale(x, y);
}

JNIEXPORT void JNICALL NEXLAYERPREFIX(scaleXYZ)(JNIEnv * env, jobject obj, jfloat x, jfloat y, jfloat z)
{
	gLayerRenderer->scale(x, y, z);
}

JNIEXPORT void JNICALL NEXLAYERPREFIX(rotateAroundAxis)(JNIEnv * env, jobject obj, jfloat a, jfloat x, jfloat y, jfloat z)
{
	gLayerRenderer->rotateAroundAxis(a, x, y, z);
}

JNIEXPORT jfloatArray JNICALL NEXLAYERPREFIX(getMatrix)(JNIEnv * env, jobject obj)
{
	jfloatArray outJNIArray;

	outJNIArray = env->NewFloatArray(16);

	env->SetFloatArrayRegion(outJNIArray, 0, 16, gLayerRenderer->getMatrix());
	
	return outJNIArray;
}

JNIEXPORT void JNICALL NEXLAYERPREFIX(rotate)(JNIEnv * env, jobject obj, jfloat a, jfloat cx, jfloat cy)
{
	gLayerRenderer->rotate(a, cx, cy);
}

JNIEXPORT void JNICALL NEXLAYERPREFIX(setAlpha)(JNIEnv * env, jobject obj, jfloat alpha)
{
	gLayerRenderer->setAlpha(alpha);
}

JNIEXPORT void JNICALL NEXLAYERPREFIX(setCurrentTime)(JNIEnv * env, jobject obj, jint currentTime)
{
	gLayerRenderer->setCurrentTime(currentTime);
}

JNIEXPORT void JNICALL NEXLAYERPREFIX(setShaderAndParam)(JNIEnv * env, jobject obj, jboolean is_export)
{
	gLayerRenderer->setShaderAndParams(is_export);
}

JNIEXPORT void JNICALL NEXLAYERPREFIX(setFrameDimensions)(JNIEnv * env, jobject obj, jint width, jint height)
{
	gLayerRenderer->setFrameDimension(width, height);
}

JNIEXPORT void JNICALL NEXLAYERPREFIX(setScreenDimenisions)(JNIEnv * env, jobject obj, jint width, jint height)
{
	gLayerRenderer->setScreenDimension(width, height);
}

JNIEXPORT void JNICALL NEXLAYERPREFIX(preRender)(JNIEnv * env, jobject obj)
{
	gLayerRenderer->preRender();
}

JNIEXPORT void JNICALL NEXLAYERPREFIX(postRender)(JNIEnv * env, jobject obj)
{
	gLayerRenderer->postRender();
}

JNIEXPORT void JNICALL NEXLAYERPREFIX(setChromakeyViewMaskEnabled)(JNIEnv * env, jobject obj, jboolean maskOnOff)
{
	gLayerRenderer->setChromakeyViewMaskEnabled(maskOnOff);
}

JNIEXPORT jint JNICALL NEXLAYERPREFIX(getChromakeyViewMaskEnabled)(JNIEnv * env, jobject obj)
{
	return gLayerRenderer->getChromakeyViewMaskEnabled();
}

JNIEXPORT void JNICALL NEXLAYERPREFIX(setChromakeyEnabled)(JNIEnv * env, jobject obj, jboolean enable)
{
	gLayerRenderer->setChromakeyEnabled(enable);
}

JNIEXPORT void JNICALL NEXLAYERPREFIX(setChromakeyColor)(JNIEnv * env, jobject obj, jint color, jfloat clipFg, jfloat clipBg, jfloat bx0, jfloat by0, jfloat bx1, jfloat by1)
{
	gLayerRenderer->setChromakeyColor(color, clipFg, clipBg, bx0, by0, bx1, by1);
}

JNIEXPORT void JNICALL NEXLAYERPREFIX(setHBlurEnabled)(JNIEnv * env, jobject obj, jboolean enable)
{
	gLayerRenderer->setHBlurEnabled(enable);
}

JNIEXPORT void JNICALL NEXLAYERPREFIX(setVBlurEnabled)(JNIEnv * env, jobject obj, jboolean enable)
{
	gLayerRenderer->setVBlurEnabled(enable);
}

JNIEXPORT void JNICALL NEXLAYERPREFIX(setMosaicEnabled)(JNIEnv * env, jobject obj, jboolean enable)
{
	gLayerRenderer->setMosaicEnabled(enable);
}

JNIEXPORT void JNICALL NEXLAYERPREFIX(setEffectStrength)(JNIEnv * env, jobject obj, jfloat strength)
{
	gLayerRenderer->setEffectStrength(strength);
}

JNIEXPORT jfloat JNICALL NEXLAYERPREFIX(getEffectStrength)(JNIEnv * env, jobject obj)
{
	return gLayerRenderer->getEffectStrength();
}

JNIEXPORT void JNICALL NEXLAYERPREFIX(setEffectTextureSize)(JNIEnv * env, jobject obj, jint width, jint height)
{
	gLayerRenderer->setEffectTextureSize(width, height);
}

JNIEXPORT jfloat JNICALL NEXLAYERPREFIX(getScreenDimensionWidth)(JNIEnv * env, jobject obj)
{
	return gLayerRenderer->getScreenDimensionWidth();
}

JNIEXPORT jfloat JNICALL NEXLAYERPREFIX(getScreenDimensionHeight)(JNIEnv * env, jobject obj)
{
	return gLayerRenderer->getScreenDimensionHeight();
}

JNIEXPORT void JNICALL NEXLAYERPREFIX(setMaskEnabled)(JNIEnv * env, jobject obj, jboolean enable)
{
	gLayerRenderer->setMaskEnabled(enable);
}

JNIEXPORT jboolean JNICALL NEXLAYERPREFIX(getMaskEnabled)(JNIEnv * env, jobject obj)
{
	return gLayerRenderer->getMaskEnabled();
}

JNIEXPORT void JNICALL NEXLAYERPREFIX(setMaskTexID)(JNIEnv * env, jobject obj, jint texid)
{
	gLayerRenderer->setMaskTexID(texid);
}

JNIEXPORT jfloat JNICALL NEXLAYERPREFIX(getAlpha)(JNIEnv * env, jobject obj)
{
	return gLayerRenderer->getAlpha();
}

JNIEXPORT void JNICALL NEXLAYERPREFIX(setAlphaTestValue)(JNIEnv * env, jobject obj, jfloat alpha)
{
	gLayerRenderer->setAlphatestValue(alpha);
}

JNIEXPORT void JNICALL NEXLAYERPREFIX(resetMatrix)(JNIEnv * env, jobject obj)
{
	gLayerRenderer->resetMatrix();
}

JNIEXPORT void JNICALL NEXLAYERPREFIX(setRenderTarget)(JNIEnv * env, jobject obj, jint rendertarget)
{
	gLayerRenderer->setRenderTarget(rendertarget);
}

JNIEXPORT void JNICALL NEXLAYERPREFIX(setZTestMode)(JNIEnv * env, jobject obj)
{
	gLayerRenderer->setZTestMode();
}

JNIEXPORT void JNICALL NEXLAYERPREFIX(setZTest)(JNIEnv * env, jobject obj)
{
	gLayerRenderer->setZTest();
}

JNIEXPORT void JNICALL NEXLAYERPREFIX(releaseZTest)(JNIEnv * env, jobject obj)
{
	gLayerRenderer->releaseZTest();
}

JNIEXPORT void JNICALL NEXLAYERPREFIX(releaseZTestMode)(JNIEnv * env, jobject obj)
{
	gLayerRenderer->releaseZTestMode();
}

JNIEXPORT void JNICALL NEXLAYERPREFIX(clearMask)(JNIEnv * env, jobject obj)
{
	gLayerRenderer->clearMask();
}

JNIEXPORT void JNICALL NEXLAYERPREFIX(clearMasktoColor)(JNIEnv * env, jobject obj, jint color)
{
	gLayerRenderer->clearMask(color);
}

JNIEXPORT void JNICALL NEXLAYERPREFIX(setRenderMode)(JNIEnv * env, jobject obj, jboolean is_export)
{
	gLayerRenderer->setRenderMode(is_export);
}

JNIEXPORT jint JNICALL NEXLAYERPREFIX(getRenderMode)(JNIEnv * env, jobject obj)
{
	return gLayerRenderer->getRenderMode();
}

JNIEXPORT void JNICALL NEXLAYERPREFIX(setLUT)(JNIEnv * env, jobject obj, jint tex_id)
{
	gLayerRenderer->setLUT(tex_id);
}

JNIEXPORT void JNICALL NEXLAYERPREFIX(setLUTEnable)(JNIEnv * env, jobject obj, jboolean enable)
{
	gLayerRenderer->setLUTEnable(enable);
}

JNIEXPORT void JNICALL NEXLAYERPREFIX(drawBitmap)(JNIEnv * env, jobject obj, jint img_id, jfloat left, jfloat top, jfloat right, jfloat bottom)
{
	gLayerRenderer->drawBitmap(img_id, left, top, right, bottom, 0);
}

JNIEXPORT void JNICALL NEXLAYERPREFIX(drawBitmapFlip)(JNIEnv * env, jobject obj, jint img_id, jfloat left, jfloat top, jfloat right, jfloat bottom, jint flipmode)
{
	gLayerRenderer->drawBitmap(img_id, left, top, right, bottom, flipmode);
}

JNIEXPORT void JNICALL NEXLAYERPREFIX(drawBitmapRepeat)(JNIEnv * env, jobject obj, jint img_id, jfloat left, jfloat top, jfloat right, jfloat bottom, jfloat repeat_x, jfloat repeat_y)
{
	gLayerRenderer->drawBitmap(img_id, left, top, right, bottom, repeat_x, repeat_y);
}

JNIEXPORT void JNICALL NEXLAYERPREFIX(drawVideo)(JNIEnv * env, jobject obj, jint clipid, jfloat x, jfloat y, jfloat w, jfloat h)
{
	gLayerRenderer->drawVideo(clipid, x, y, w, h);
}

JNIEXPORT void JNICALL NEXLAYERPREFIX(drawDirect)(JNIEnv * env, jobject obj, jint texname, jint type, jfloat x, jfloat y, jfloat w, jfloat h, jint flipmode)
{
	gLayerRenderer->drawDirect(texname, type, x, y, w, h, flipmode);
}

JNIEXPORT void JNICALL NEXLAYERPREFIX(drawDirectFlip)(JNIEnv * env, jobject obj, jint texname, jfloat x, jfloat y, jfloat w, jfloat h, jint flipmode)
{
	gLayerRenderer->drawDirect(texname, 0, x, y, w, h, flipmode);
}

JNIEXPORT void JNICALL NEXLAYERPREFIX(drawRenderItem)(JNIEnv * env, jobject obj, jint effect_id, jstring usersettings, jint current_time, jint start_time, jint end_time, jfloat left, jfloat top, jfloat right, jfloat bottom, jfloat alpha, jboolean mask_enabled)
{
	const char *cstr_usersettings = env->GetStringUTFChars(usersettings, NULL);
	gLayerRenderer->drawRenderItem(effect_id, (char*)cstr_usersettings, current_time, start_time, end_time, left, top, right, bottom, alpha, mask_enabled, 0);
	if(cstr_usersettings)
		env->ReleaseStringUTFChars(usersettings, cstr_usersettings);
}

JNIEXPORT void JNICALL NEXLAYERPREFIX(drawRenderItemFlip)(JNIEnv * env, jobject obj, jint effect_id, jstring usersettings, jint current_time, jint start_time, jint end_time, jfloat left, jfloat top, jfloat right, jfloat bottom, jfloat alpha, jboolean mask_enabled, jint flipmode)
{
	const char *cstr_usersettings = env->GetStringUTFChars(usersettings, NULL);
	gLayerRenderer->drawRenderItem(effect_id, (char*)cstr_usersettings, current_time, start_time, end_time, left, top, right, bottom, alpha, mask_enabled, flipmode);
	if(cstr_usersettings)
		env->ReleaseStringUTFChars(usersettings, cstr_usersettings);
}

JNIEXPORT void JNICALL NEXLAYERPREFIX(drawRenderItemBlend)(JNIEnv * env, jobject obj, jint effect_id, jint tex_id_for_second, jint type, jstring usersettings, jint current_time, jint start_time, jint end_time, jfloat x, jfloat y, jfloat w, jfloat h, jfloat alpha, jboolean mask_enabled)
{
	const char *cstr_usersettings = env->GetStringUTFChars(usersettings, NULL);
	gLayerRenderer->drawRenderItemBlend(effect_id, tex_id_for_second, type, (char*)cstr_usersettings, current_time, start_time, end_time, x, y, w, h, alpha, mask_enabled, 0);
	if(cstr_usersettings)
		env->ReleaseStringUTFChars(usersettings, cstr_usersettings);
}

JNIEXPORT void JNICALL NEXLAYERPREFIX(drawRenderItemBlendOverlay)(JNIEnv * env, jobject obj, jint effect_id_for_blend, jint effect_id, jstring usersettings_for_blend, jstring usersettings, jint current_time, jint start_time, jint end_time, jfloat left, jfloat top, jfloat right, jfloat bottom, jfloat alpha, jboolean mask_enabled)
{
	const char *cstr_usersettings_for_blend = env->GetStringUTFChars(usersettings_for_blend, NULL);
	const char *cstr_usersettings = env->GetStringUTFChars(usersettings, NULL);
	
	gLayerRenderer->drawRenderItemBlendOverlay(effect_id_for_blend, effect_id, (char*)cstr_usersettings_for_blend, (char*)cstr_usersettings, current_time, start_time, end_time, left, top, right, bottom, alpha, mask_enabled, 0);
	if(cstr_usersettings_for_blend)
		env->ReleaseStringUTFChars(usersettings_for_blend, cstr_usersettings_for_blend);
	if(cstr_usersettings)
		env->ReleaseStringUTFChars(usersettings, cstr_usersettings);
}

JNIEXPORT void JNICALL NEXLAYERPREFIX(drawRenderItemBlendFlip)(JNIEnv * env, jobject obj, jint effect_id, jint tex_id_for_second, jstring usersettings, jint current_time, jint start_time, jint end_time, jfloat x, jfloat y, jfloat w, jfloat h, jfloat alpha, jboolean mask_enabled, jint flipmode)
{
	const char *cstr_usersettings = env->GetStringUTFChars(usersettings, NULL);
	gLayerRenderer->drawRenderItemBlend(effect_id, tex_id_for_second, 0, (char*)cstr_usersettings, current_time, start_time, end_time, x, y, w, h, alpha, mask_enabled, flipmode);
	if(cstr_usersettings)
		env->ReleaseStringUTFChars(usersettings, cstr_usersettings);
}

JNIEXPORT void JNICALL NEXLAYERPREFIX(drawRenderItemBitmap)(JNIEnv * env, jobject obj, jint effect_id, jint tex_id_for_second, jstring usersettings, jint current_time, jint start_time, jint end_time, jfloat left, jfloat top, jfloat right, jfloat bottom, jfloat alpha, jboolean mask_enabled)
{
	const char *cstr_usersettings = env->GetStringUTFChars(usersettings, NULL);
	gLayerRenderer->drawRenderItemBitmap(effect_id, tex_id_for_second, (char*)cstr_usersettings, current_time, start_time, end_time, left, top, right, bottom, alpha, mask_enabled, 0);
	if(cstr_usersettings)
		env->ReleaseStringUTFChars(usersettings, cstr_usersettings);
}

JNIEXPORT void JNICALL NEXLAYERPREFIX(drawRenderItemBitmapFlip)(JNIEnv * env, jobject obj, jint effect_id, jint tex_id_for_second, jstring usersettings, jint current_time, jint start_time, jint end_time, jfloat left, jfloat top, jfloat right, jfloat bottom, jfloat alpha, jboolean mask_enabled, jint flipmode)
{
	const char *cstr_usersettings = env->GetStringUTFChars(usersettings, NULL);
	gLayerRenderer->drawRenderItemBitmap(effect_id, tex_id_for_second, (char*)cstr_usersettings, current_time, start_time, end_time, left, top, right, bottom, alpha, mask_enabled, flipmode);
	if(cstr_usersettings)
		env->ReleaseStringUTFChars(usersettings, cstr_usersettings);
}

JNIEXPORT void JNICALL NEXLAYERPREFIX(fillRect)(JNIEnv * env, jobject obj, jint fill_rect_tex_id, jint color, jfloat left, jfloat top, jfloat right, jfloat bottom)
{
	gLayerRenderer->fillRect(fill_rect_tex_id, color, left, top, right, bottom);
}

JNIEXPORT void JNICALL NEXLAYERPREFIX(clearMaskRegion)(JNIEnv * env, jobject obj, jint fill_rect_tex_id, jint color, jfloat left, jfloat top, jfloat right, jfloat bottom)
{
	gLayerRenderer->clearMaskRegion(fill_rect_tex_id, color, left, top, right, bottom);
}

JNIEXPORT void JNICALL NEXLAYERPREFIX(presetForDrawing)(JNIEnv * env, jobject obj, jfloat left, jfloat top, jfloat right, jfloat bottom, jint type)
{
	gLayerRenderer->presetForDrawing(left, top, right, bottom, type);
}

JNIEXPORT jint JNICALL NEXLAYERPREFIX(getRenderTarget)(JNIEnv * env, jobject obj)
{
	return gLayerRenderer->getRenderTarget();
}

JNIEXPORT void JNICALL NEXLAYERPREFIX(setColorLookupTable)(JNIEnv * env, jobject obj, jint img_id, jfloat x, jfloat y)
{
	gLayerRenderer->setColorLookupTable(img_id, x, y);
}

JNIEXPORT void JNICALL NEXLAYERPREFIX(setHue)(JNIEnv * env, jobject obj, jfloat hue)
{
	gLayerRenderer->setHue(hue);
}

JNIEXPORT jfloat JNICALL NEXLAYERPREFIX(getHue)(JNIEnv * env, jobject obj)
{
	return gLayerRenderer->getHue();
}

JNIEXPORT void JNICALL NEXLAYERPREFIX(drawNexEDL)(JNIEnv * env, jobject obj, jint effect_id, jstring usersettings, jint current_time, jint start_time, jint end_time, jfloat left, jfloat top, jfloat right, jfloat bottom, jfloat alpha, jint mode)
{
	const char *cstr_usersettings = env->GetStringUTFChars(usersettings, NULL);
	gLayerRenderer->drawNexEDL(effect_id, (char*)cstr_usersettings, current_time, start_time, end_time, left, top, right, bottom, alpha, mode);
	if(cstr_usersettings)
		env->ReleaseStringUTFChars(usersettings, cstr_usersettings);
}

JNIEXPORT void JNICALL NEXLAYERPREFIX(setBlendFuncNormal)(JNIEnv * env, jobject obj)
{
	gLayerRenderer->setBlendFuncNormal();
}

JNIEXPORT void JNICALL NEXLAYERPREFIX(setBlendFuncMultiply)(JNIEnv * env, jobject obj)
{
	gLayerRenderer->setBlendFuncMultiply();
}