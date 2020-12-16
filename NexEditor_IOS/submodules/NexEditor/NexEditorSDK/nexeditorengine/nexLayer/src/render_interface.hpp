#pragma once

class RendererInterface{

public:
	virtual unsigned int getLUT() = 0;
	virtual int getChromakeyViewMaskEnabled() = 0;
	virtual float getEffectStrength() = 0;
	virtual int getEffectTextureWidth() = 0;
	virtual int getEffectTextureHeight() = 0;
	virtual int getScreenDimensionWidth() = 0;
	virtual int getScreenDimensionHeight() = 0;
	virtual int getFrameWidth() = 0;
	virtual int getFrameHeight() = 0;
	virtual unsigned int getMaskTexID() = 0;
	virtual int getCurrentTime() = 0;
	virtual float getAlpha() = 0;
	virtual float getAlphatestValue() = 0;
	virtual void setHue(float hue) = 0;
	virtual float getHue() = 0;
	virtual ChromakeyInfo& getChromakeyColor() = 0;
};