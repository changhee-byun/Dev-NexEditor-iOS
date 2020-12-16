#pragma once

class StateStackEntry {

	float matrix_[16];
	float alpha_;
	float scale_x_, scale_y_;
	int cts_;
	float hue_;
	ColorMatrix color_matrix_;
	int render_target_;
	bool mask_enabled_;

public:
	void copyFrom(float* pmatrix, float alpha, int cts, ColorMatrix colorMatrix, int renderTarget, bool maskEnabled, float scale_x, float scale_y, float hue) {
		
		memcpy(matrix_, pmatrix, sizeof(matrix_));
		alpha_ = alpha;
		cts_ = cts;
		color_matrix_.set(colorMatrix);
		mask_enabled_ = maskEnabled;
		render_target_ = renderTarget;
		scale_x_ = scale_x;
		scale_y_ = scale_y;
		hue_ = hue;
	}

	float* getMatrix() {

		return matrix_;
	}

	ColorMatrix& getColorMatrix() {

		return color_matrix_;
	}

	float getAlpha() {

		return alpha_;
	}

	int getCTS() {

		return cts_;
	}
	
	int getRenderTarget() {

		return render_target_;
	}

	bool getMaskEnabled() {

		return mask_enabled_;
	}
	float getScaleX() {

		return scale_x_;
	}

    float getScaleY() { 

		return  scale_y_;
	}

	float getHue() {

		return hue_;
	}
};